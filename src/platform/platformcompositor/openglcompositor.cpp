/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QWindow>
#include <QtGui/QMatrix4x4>
#include <QtGui/qpa/qplatformbackingstore.h>

#include "openglcompositor.h"

namespace GreenIsland {

namespace Platform {

/*!
    \class QOpenGLCompositor
    \brief A generic OpenGL-based compositor
    \since 5.4
    \internal
    \ingroup qpa

    This class provides a lightweight compositor that maintains the
    basic stacking order of windows and composites them by drawing
    textured quads via OpenGL.

    It it meant to be used by platform plugins that run without a
    windowing system.

    It is up to the platform plugin to manage the lifetime of the
    compositor (instance(), destroy()), set the correct destination
    context and window as early as possible (setTargetWindow()),
    register the composited windows as they are shown, activated,
    raised and lowered (addWindow(), moveToTop(), etc.), and to
    schedule repaints (update()).

    \note To get support for QWidget-based windows, just use
    QOpenGLCompositorBackingStore. It will automatically create
    textures from the raster-rendered content and trigger the
    necessary repaints.
 */

static OpenGLCompositor *compositor = 0;

OpenGLCompositor::OpenGLCompositor()
    : m_context(0),
      m_targetWindow(0)
{
    Q_ASSERT(!compositor);
    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(0);
    connect(&m_updateTimer, SIGNAL(timeout()), SLOT(handleRenderAllRequest()));
}

OpenGLCompositor::~OpenGLCompositor()
{
    Q_ASSERT(compositor == this);
    m_blitter.destroy();
    compositor = 0;
}

void OpenGLCompositor::setTarget(QOpenGLContext *context, QWindow *targetWindow)
{
    m_context = context;
    m_targetWindow = targetWindow;
}

void OpenGLCompositor::update()
{
    if (!m_updateTimer.isActive())
        m_updateTimer.start();
}

QImage OpenGLCompositor::grab()
{
    Q_ASSERT(m_context && m_targetWindow);
    m_context->makeCurrent(m_targetWindow);
    QScopedPointer<QOpenGLFramebufferObject> fbo(new QOpenGLFramebufferObject(m_targetWindow->geometry().size()));
    renderAll(fbo.data());
    return fbo->toImage();
}

void OpenGLCompositor::handleRenderAllRequest()
{
    Q_ASSERT(m_context && m_targetWindow);
    m_context->makeCurrent(m_targetWindow);
    renderAll(0);
}

void OpenGLCompositor::renderAll(QOpenGLFramebufferObject *fbo)
{
    if (fbo)
        fbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const QRect targetWindowRect(QPoint(0, 0), m_targetWindow->geometry().size());
    glViewport(0, 0, targetWindowRect.width(), targetWindowRect.height());

    if (!m_blitter.isCreated())
        m_blitter.create();

    m_blitter.bind();

    for (int i = 0; i < m_windows.size(); ++i)
        m_windows.at(i)->beginCompositing();

    for (int i = 0; i < m_windows.size(); ++i)
        render(m_windows.at(i));

    m_blitter.release();
    if (!fbo)
        m_context->swapBuffers(m_targetWindow);
    else
        fbo->release();

    for (int i = 0; i < m_windows.size(); ++i)
        m_windows.at(i)->endCompositing();
}

struct BlendStateBinder
{
    BlendStateBinder() : m_blend(false) {
        glDisable(GL_BLEND);
    }
    void set(bool blend) {
        if (blend != m_blend) {
            if (blend) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } else {
                glDisable(GL_BLEND);
            }
            m_blend = blend;
        }
    }
    ~BlendStateBinder() {
        if (m_blend)
            glDisable(GL_BLEND);
    }
    bool m_blend;
};

static inline QRect toBottomLeftRect(const QRect &topLeftRect, int windowHeight)
{
    return QRect(topLeftRect.x(), windowHeight - topLeftRect.bottomRight().y() - 1,
                 topLeftRect.width(), topLeftRect.height());
}

static void clippedBlit(const QPlatformTextureList *textures, int idx, const QRect &targetWindowRect, QOpenGLTextureBlitter *blitter)
{
    const QRect rectInWindow = textures->geometry(idx);
    QRect clipRect = textures->clipRect(idx);
    if (clipRect.isEmpty())
        clipRect = QRect(QPoint(0, 0), rectInWindow.size());

    const QRect clippedRectInWindow = rectInWindow & clipRect.translated(rectInWindow.topLeft());
    const QRect srcRect = toBottomLeftRect(clipRect, rectInWindow.height());

    const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(clippedRectInWindow, targetWindowRect);
    const QMatrix3x3 source = QOpenGLTextureBlitter::sourceTransform(srcRect, rectInWindow.size(),
                                                                     QOpenGLTextureBlitter::OriginBottomLeft);

    blitter->blit(textures->textureId(idx), target, source);
}

void OpenGLCompositor::render(OpenGLCompositorWindow *window)
{
    const QPlatformTextureList *textures = window->textures();
    if (!textures)
        return;

    const QRect targetWindowRect(QPoint(0, 0), m_targetWindow->geometry().size());
    float currentOpacity = 1.0f;
    BlendStateBinder blend;

    for (int i = 0; i < textures->count(); ++i) {
        uint textureId = textures->textureId(i);
        const float opacity = window->sourceWindow()->opacity();
        if (opacity != currentOpacity) {
            currentOpacity = opacity;
            m_blitter.setOpacity(currentOpacity);
        }

        if (textures->count() > 1 && i == textures->count() - 1) {
            // Backingstore for a widget with QOpenGLWidget subwidgets
            blend.set(true);
            const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(textures->geometry(i), targetWindowRect);
            m_blitter.blit(textureId, target, QOpenGLTextureBlitter::OriginTopLeft);
        } else if (textures->count() == 1) {
            // A regular QWidget window
            const bool translucent = window->sourceWindow()->requestedFormat().alphaBufferSize() > 0;
            blend.set(translucent);
            const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(textures->geometry(i), targetWindowRect);
            m_blitter.blit(textureId, target, QOpenGLTextureBlitter::OriginTopLeft);
        } else if (!textures->flags(i).testFlag(QPlatformTextureList::StacksOnTop)) {
            // Texture from an FBO belonging to a QOpenGLWidget
            blend.set(false);
            clippedBlit(textures, i, targetWindowRect, &m_blitter);
        }
    }

    for (int i = 0; i < textures->count(); ++i) {
        if (textures->flags(i).testFlag(QPlatformTextureList::StacksOnTop)) {
            blend.set(true);
            clippedBlit(textures, i, targetWindowRect, &m_blitter);
        }
    }

    m_blitter.setOpacity(1.0f);
}

OpenGLCompositor *OpenGLCompositor::instance()
{
    if (!compositor)
        compositor = new OpenGLCompositor;
    return compositor;
}

void OpenGLCompositor::destroy()
{
    delete compositor;
    compositor = 0;
}

void OpenGLCompositor::addWindow(OpenGLCompositorWindow *window)
{
    if (!m_windows.contains(window)) {
        m_windows.append(window);
        emit topWindowChanged(window);
    }
}

void OpenGLCompositor::removeWindow(OpenGLCompositorWindow *window)
{
    m_windows.removeOne(window);
    if (!m_windows.isEmpty())
        emit topWindowChanged(m_windows.last());
}

void OpenGLCompositor::moveToTop(OpenGLCompositorWindow *window)
{
    m_windows.removeOne(window);
    m_windows.append(window);
    emit topWindowChanged(window);
}

void OpenGLCompositor::changeWindowIndex(OpenGLCompositorWindow *window, int newIdx)
{
    int idx = m_windows.indexOf(window);
    if (idx != -1 && idx != newIdx) {
        m_windows.move(idx, newIdx);
        if (newIdx == m_windows.size() - 1)
            emit topWindowChanged(m_windows.last());
    }
}

} // namespace Platform

} // namespace GreenIsland

#include "moc_openglcompositor.cpp"
