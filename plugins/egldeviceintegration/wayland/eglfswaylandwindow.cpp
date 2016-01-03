/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL2.1+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
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

#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLFramebufferObject>

#include <GreenIsland/Platform/EGLConvenience>
#include <GreenIsland/Platform/EglFSIntegration>

#include <GreenIsland/client/private/surface_p.h>

#include "eglfswaylandlogging.h"
#include "eglfswaylandscreen.h"
#include "eglfswaylandwindow.h"

#include <wayland-egl.h>

using namespace GreenIsland::Client;

namespace GreenIsland {

namespace Platform {

static WId newWId()
{
    static WId id = 0;

    if (id == std::numeric_limits<WId>::max())
        qCWarning(gLcEglFSWayland, "EglFSWaylandWindow: Out of window IDs");

    return ++id;
}

EglFSWaylandWindow::EglFSWaylandWindow(EglFSWaylandIntegration *integration,
                                       QWindow *window)
    : QObject(integration)
    , QPlatformWindow(window)
    , m_integration(integration)
    , m_winId(0)
    , m_eglDisplay(EGL_NO_DISPLAY)
    , m_eglConfig(Q_NULLPTR)
    , m_eglWindow(Q_NULLPTR)
    , m_eglSurface(EGL_NO_SURFACE)
    , m_contentFBO(Q_NULLPTR)
    , m_created(false)
    , m_resize(false)
{
    m_surface = m_integration->compositor()->createSurface(this);

    EglFSWaylandScreen *nativeScreen = static_cast<EglFSWaylandScreen *>(screen());
    m_output = nativeScreen->output();
}

EglFSWaylandWindow::~EglFSWaylandWindow()
{
    destroy();

    m_surface->deleteLater();

    if (m_eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(m_eglDisplay, m_eglSurface);
        m_eglSurface = EGL_NO_SURFACE;
    }

    if (m_eglWindow) {
        wl_egl_window_destroy(m_eglWindow);
        m_eglWindow = Q_NULLPTR;
    }

    delete m_contentFBO;
}

WId EglFSWaylandWindow::winId() const
{
    return m_winId;
}

GLuint EglFSWaylandWindow::contentFBO()
{
    if (m_resize || !m_contentFBO) {
        QOpenGLFramebufferObject *old = m_contentFBO;
        QSize size = geometry().size() * scale();
        m_contentFBO = new QOpenGLFramebufferObject(
                    size.width(), size.height(),
                    QOpenGLFramebufferObject::CombinedDepthStencil);
        delete old;
        m_resize = false;
    }

    return m_contentFBO->handle();
}

GLuint EglFSWaylandWindow::contentTexture() const
{
    return m_contentFBO->texture();
}

bool EglFSWaylandWindow::needsToUpdateContentFBO() const
{
    return m_resize || !m_contentFBO;
}

void EglFSWaylandWindow::bindContentFBO()
{
    contentFBO();
    m_contentFBO->bind();
}

void EglFSWaylandWindow::create()
{
    if (m_created)
        return;

    m_winId = newWId();

    m_created = true;

    setGeometry(QRect());
    QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(0, 0), geometry().size()));

    EglFSWaylandScreen *nativeScreen = static_cast<EglFSWaylandScreen *>(screen());
    m_eglDisplay = nativeScreen->display();
    QSurfaceFormat platformFormat = egl_device_integration()->surfaceFormatFor(window()->requestedFormat());
    m_eglConfig = EglFSIntegration::chooseConfig(m_eglDisplay, platformFormat);
    m_format = EglUtils::glFormatFromConfig(m_eglDisplay, m_eglConfig, platformFormat);

    resetSurface();

    handleContentOrientationChange(window()->contentOrientation());
}

void EglFSWaylandWindow::destroy()
{
    invalidateSurface();
    m_created = false;
}

void EglFSWaylandWindow::unmap()
{
    m_surface->attach(BufferPtr(), QPoint(0, 0));
    m_surface->commit(Client::Surface::NoCommitMode);
}

void EglFSWaylandWindow::invalidateSurface()
{
    if (m_eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(m_eglDisplay, m_eglSurface);
        m_eglSurface = EGL_NO_SURFACE;
    }

    if (m_eglWindow) {
        wl_egl_window_destroy(m_eglWindow);
        m_eglWindow = Q_NULLPTR;
    }
}

void EglFSWaylandWindow::resetSurface()
{
    QSize size = geometry().size();
    m_eglWindow = wl_egl_window_create(SurfacePrivate::get(m_surface)->object(),
                                       size.width(), size.height());
    EGLNativeWindowType nativeWindow = reinterpret_cast<EGLNativeWindowType>(m_eglWindow);
    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig,
                                          nativeWindow, Q_NULLPTR);
    if (m_eglSurface == EGL_NO_SURFACE) {
        EGLint error = eglGetError();
        eglTerminate(m_eglDisplay);
        qFatal("EGL Error : Could not create the egl surface: error = 0x%x\n", error);
    }
}

void EglFSWaylandWindow::setGeometry(const QRect &rect)
{
    if (m_integration->fullScreenShell())
        QPlatformWindow::setGeometry(m_output->geometry());
    else
        QPlatformWindow::setGeometry(QRect(rect.x(), rect.y(),
                                           qBound(window()->minimumWidth(), rect.width(), window()->maximumWidth()),
                                           qBound(window()->minimumHeight(), rect.height(), window()->maximumHeight())));

    if (m_contentFBO) {
        if (m_contentFBO->width() != geometry().width() || m_contentFBO->height() != geometry().height())
            m_resize = true;
    }

    if (window()->isVisible() && rect.isValid())
        QWindowSystemInterface::handleGeometryChange(window(), geometry());
    QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), geometry().size()));

    if (m_eglWindow)
        wl_egl_window_resize(m_eglWindow, geometry().width(), geometry().height(), 0, 0);
}

bool EglFSWaylandWindow::isExposed() const
{
    return window()->isVisible();
}

int EglFSWaylandWindow::scale() const
{
    return m_output->scale();
}

QRect EglFSWaylandWindow::contentsRect() const
{
    return geometry();
}

void EglFSWaylandWindow::setVisible(bool visible)
{
    if (!visible) {
        QWindowSystemInterface::handleExposeEvent(window(), QRegion());
        unmap();
        invalidateSurface();
    }

    QPlatformWindow::setVisible(visible);
}

void EglFSWaylandWindow::windowEvent(QEvent *event)
{
    if (QEvent::Expose) {
        if (isExposed()) {
            if (m_integration->fullScreenShell())
                m_integration->fullScreenShell()->presentSurface(m_surface, m_output);
        } else {
            if (m_integration->fullScreenShell())
                m_integration->fullScreenShell()->hideOutput(m_output);
        }
    }

    QPlatformWindow::windowEvent(event);
}

void EglFSWaylandWindow::handleContentOrientationChange(Qt::ScreenOrientation orientation)
{
    bool isPortrait = window()->screen() && window()->screen()->primaryOrientation() == Qt::PortraitOrientation;
    Output::Transform transform;

    switch (orientation) {
    case Qt::PrimaryOrientation:
        transform = Output::TransformNormal;
        break;
    case Qt::LandscapeOrientation:
        transform = isPortrait ? Output::Transform270 : Output::TransformNormal;
        break;
    case Qt::PortraitOrientation:
        transform = isPortrait ? Output::TransformNormal : Output::Transform90;
        break;
    case Qt::InvertedLandscapeOrientation:
        transform = isPortrait ? Output::Transform90 : Output::Transform180;
        break;
    case Qt::InvertedPortraitOrientation:
        transform = isPortrait ? Output::Transform180 : Output::Transform270;
        break;
    default:
        Q_UNREACHABLE();
    }

    m_surface->setBufferTransform(transform);
    m_surface->commit();
}

EglFSWaylandWindow *EglFSWaylandWindow::fromSurface(Client::Surface *surface)
{
    Q_FOREACH (QWindow *window, QGuiApplication::topLevelWindows()) {
        EglFSWaylandWindow *w =
                static_cast<EglFSWaylandWindow *>(window->handle());
        if (w && w->waylandSurface() == surface)
            return w;
    }
    return Q_NULLPTR;
}

} // namespace Platform

} // namespace GreenIsland

#include "moc_eglfswaylandwindow.cpp"
