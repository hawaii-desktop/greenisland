/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef GREENISLAND_EGLFSWAYLANDWINDOW_H
#define GREENISLAND_EGLFSWAYLANDWINDOW_H

#include <QtCore/QObject>
#include <QtGui/qpa/qplatformwindow.h>

#include <GreenIsland/Client/Surface>

#include <GreenIsland/Platform/EGLPlatformContext>

#include "eglfswaylandintegration.h"

class QOpenGLFramebufferObject;

struct wl_egl_window;

namespace GreenIsland {

namespace Platform {

class EglFSWaylandWindow : public QObject, public QPlatformWindow
{
    Q_OBJECT
public:
    EglFSWaylandWindow(EglFSWaylandIntegration *integration, QWindow *window);
    ~EglFSWaylandWindow();

    WId winId() const Q_DECL_OVERRIDE;

    Client::Surface *waylandSurface() const { return m_surface; }
    EGLSurface surface() const { return m_eglSurface; }

    GLuint contentFBO();
    GLuint contentTexture() const;

    bool needsToUpdateContentFBO() const;

    void bindContentFBO();

    void create();
    void destroy();
    void unmap();

    void invalidateSurface() Q_DECL_OVERRIDE;
    virtual void resetSurface();

    void setGeometry(const QRect &rect) Q_DECL_OVERRIDE;

    bool isExposed() const Q_DECL_OVERRIDE;
    int scale() const;
    QRect contentsRect() const;

    void setVisible(bool visible) Q_DECL_OVERRIDE;

    void windowEvent(QEvent *event) Q_DECL_OVERRIDE;

    void handleContentOrientationChange(Qt::ScreenOrientation orientation) Q_DECL_OVERRIDE;

    static EglFSWaylandWindow *fromSurface(Client::Surface *surface);

private:
    EglFSWaylandIntegration *m_integration;
    WId m_winId;
    Client::Surface *m_surface;
    Client::Output *m_output;

    EGLDisplay m_eglDisplay;
    EGLConfig m_eglConfig;
    QSurfaceFormat m_format;
    wl_egl_window *m_eglWindow;
    EGLSurface m_eglSurface;
    QOpenGLFramebufferObject *m_contentFBO;

    bool m_created;
    mutable bool m_resize;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSWAYLANDWINDOW_H
