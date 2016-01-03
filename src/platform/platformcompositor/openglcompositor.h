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

#ifndef OPENGLCOMPOSITOR_H
#define OPENGLCOMPOSITOR_H

#include <QtCore/QTimer>
#include <QtGui/private/qopengltextureblitter_p.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

class QOpenGLContext;
class QOpenGLFramebufferObject;
class QWindow;
class QPlatformTextureList;

namespace GreenIsland {

namespace Platform {

class GREENISLANDPLATFORM_EXPORT OpenGLCompositorWindow
{
public:
    virtual ~OpenGLCompositorWindow() { }
    virtual QWindow *sourceWindow() const = 0;
    virtual const QPlatformTextureList *textures() const = 0;
    virtual void beginCompositing() { }
    virtual void endCompositing() { }
};

class GREENISLANDPLATFORM_EXPORT OpenGLCompositor : public QObject
{
    Q_OBJECT
public:
    static OpenGLCompositor *instance();
    static void destroy();

    void setTarget(QOpenGLContext *context, QWindow *window);
    QOpenGLContext *context() const { return m_context; }
    QWindow *targetWindow() const { return m_targetWindow; }

    void update();
    QImage grab();

    QList<OpenGLCompositorWindow *> windows() const { return m_windows; }
    void addWindow(OpenGLCompositorWindow *window);
    void removeWindow(OpenGLCompositorWindow *window);
    void moveToTop(OpenGLCompositorWindow *window);
    void changeWindowIndex(OpenGLCompositorWindow *window, int newIdx);

Q_SIGNALS:
    void topWindowChanged(OpenGLCompositorWindow *window);

private Q_SLOTS:
    void handleRenderAllRequest();

private:
    OpenGLCompositor();
    ~OpenGLCompositor();

    void renderAll(QOpenGLFramebufferObject *fbo);
    void render(OpenGLCompositorWindow *window);

    QOpenGLContext *m_context;
    QWindow *m_targetWindow;
    QTimer m_updateTimer;
    QOpenGLTextureBlitter m_blitter;
    QList<OpenGLCompositorWindow *> m_windows;
};

} // namespace Platform

} // namespace GreenIsland

#endif // OPENGLCOMPOSITOR_H
