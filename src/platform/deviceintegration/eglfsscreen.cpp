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

#include <QtCore/qtextstream.h>
#include <QtGui/qwindow.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtGui/qpa/qplatformcursor.h>

#include "logging.h"
#include "deviceintegration/egldeviceintegration.h"
#include "deviceintegration/eglfsscreen.h"
#include "deviceintegration/eglfswindow.h"
#include "platformcompositor/openglcompositor.h"

namespace GreenIsland {

namespace Platform {

EglFSScreen::EglFSScreen(EGLDisplay dpy)
    : m_dpy(dpy),
      m_pointerWindow(0),
      m_surface(EGL_NO_SURFACE),
      m_cursor(0)
{
    m_cursor = egl_device_integration()->createCursor(this);
}

EglFSScreen::~EglFSScreen()
{
    delete m_cursor;
    OpenGLCompositor::destroy();
}

QRect EglFSScreen::geometry() const
{
    return QRect(QPoint(0, 0), egl_device_integration()->screenSize());
}

int EglFSScreen::depth() const
{
    return egl_device_integration()->screenDepth();
}

QImage::Format EglFSScreen::format() const
{
    return egl_device_integration()->screenFormat();
}

QSizeF EglFSScreen::physicalSize() const
{
    return egl_device_integration()->physicalScreenSize();
}

QDpi EglFSScreen::logicalDpi() const
{
    return egl_device_integration()->logicalDpi();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
qreal EglFSScreen::pixelDensity() const
{
    return egl_device_integration()->pixelDensity();
}
#endif

Qt::ScreenOrientation EglFSScreen::nativeOrientation() const
{
    return egl_device_integration()->nativeOrientation();
}

Qt::ScreenOrientation EglFSScreen::orientation() const
{
    return egl_device_integration()->orientation();
}

QPlatformCursor *EglFSScreen::cursor() const
{
    return m_cursor;
}

qreal EglFSScreen::refreshRate() const
{
    return egl_device_integration()->refreshRate();
}

void EglFSScreen::setPrimarySurface(EGLSurface surface)
{
    m_surface = surface;
}

void EglFSScreen::handleCursorMove(const QPoint &pos)
{
    const OpenGLCompositor *compositor = OpenGLCompositor::instance();
    const QList<OpenGLCompositorWindow *> windows = compositor->windows();

    // Generate enter and leave events like a real windowing system would do.
    if (windows.isEmpty())
        return;

    // First window is always fullscreen.
    if (windows.count() == 1) {
        QWindow *window = windows[0]->sourceWindow();
        if (m_pointerWindow != window) {
            m_pointerWindow = window;
            QWindowSystemInterface::handleEnterEvent(window, window->mapFromGlobal(pos), pos);
        }
        return;
    }

    QWindow *enter = 0, *leave = 0;
    for (int i = windows.count() - 1; i >= 0; --i) {
        QWindow *window = windows[i]->sourceWindow();
        const QRect geom = window->geometry();
        if (geom.contains(pos)) {
            if (m_pointerWindow != window) {
                leave = m_pointerWindow;
                m_pointerWindow = window;
                enter = window;
            }
            break;
        }
    }

    if (enter && leave)
        QWindowSystemInterface::handleEnterLeaveEvent(enter, leave, enter->mapFromGlobal(pos), pos);
}

QPixmap EglFSScreen::grabWindow(WId wid, int x, int y, int width, int height) const
{
    OpenGLCompositor *compositor = OpenGLCompositor::instance();
    const QList<OpenGLCompositorWindow *> windows = compositor->windows();
    Q_ASSERT(!windows.isEmpty());

    QImage img;

    if (static_cast<EglFSWindow *>(windows.first()->sourceWindow()->handle())->isRaster()) {
        // Request the compositor to render everything into an FBO and read it back. This
        // is of course slow, but it's safe and reliable. It will not include the mouse
        // cursor, which is a plus.
        img = compositor->grab();
    } else {
        // Just a single OpenGL window without compositing. Do not support this case for now. Doing
        // glReadPixels is not an option since it would read from the back buffer which may have
        // undefined content when calling right after a swapBuffers (unless preserved swap is
        // available and enabled, but we have no support for that).
        qCWarning(lcDeviceIntegration)
                << "grabWindow: Not supported for non-composited OpenGL content."
                << "Use QQuickWindow::grabWindow() instead.";
        return QPixmap();
    }

    if (!wid) {
        const QSize screenSize = geometry().size();
        if (width < 0)
            width = screenSize.width() - x;
        if (height < 0)
            height = screenSize.height() - y;
        return QPixmap::fromImage(img).copy(x, y, width, height);
    }

    foreach (OpenGLCompositorWindow *w, windows) {
        const QWindow *window = w->sourceWindow();
        if (window->winId() == wid) {
            const QRect geom = window->geometry();
            if (width < 0)
                width = geom.width() - x;
            if (height < 0)
                height = geom.height() - y;
            QRect rect(geom.topLeft() + QPoint(x, y), QSize(width, height));
            rect &= window->geometry();
            return QPixmap::fromImage(img).copy(rect);
        }
    }

    return QPixmap();
}

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
/*!
  Returns the current power state.

  The default implementation always returns PowerStateOn.
*/
EglFSScreen::PowerState EglFSScreen::powerState() const
{
    return PowerStateOn;
}

/*!
  Sets the power state for this screen.
*/
void EglFSScreen::setPowerState(PowerState state)
{
    Q_UNUSED(state);
}
#endif

} // namespace Platform

} // namespace GreenIsland
