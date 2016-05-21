/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
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
