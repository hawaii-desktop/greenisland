/****************************************************************************
**
** Copyright (C) 2015-2016 Pier Luigi Fiorini
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef GREENISLAND_EGLFSSCREEN_H
#define GREENISLAND_EGLFSSCREEN_H

#include <QtGui/qpa/qplatformscreen.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

#include <EGL/egl.h>

class QOpenGLContext;

namespace GreenIsland {

namespace Platform {

class EglFSWindow;

class GREENISLANDPLATFORM_EXPORT EglFSScreen : public QPlatformScreen
{
public:
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
    enum PowerState {
        PowerStateOn,
        PowerStateStandby,
        PowerStateSuspend,
        PowerStateOff
    };
#endif

    EglFSScreen(EGLDisplay display);
    ~EglFSScreen();

    QRect geometry() const Q_DECL_OVERRIDE;
    int depth() const Q_DECL_OVERRIDE;
    QImage::Format format() const Q_DECL_OVERRIDE;

    QSizeF physicalSize() const Q_DECL_OVERRIDE;
    QDpi logicalDpi() const Q_DECL_OVERRIDE;
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    qreal pixelDensity() const Q_DECL_OVERRIDE;
#endif
    Qt::ScreenOrientation nativeOrientation() const Q_DECL_OVERRIDE;
    Qt::ScreenOrientation orientation() const Q_DECL_OVERRIDE;

    QPlatformCursor *cursor() const Q_DECL_OVERRIDE;

    qreal refreshRate() const Q_DECL_OVERRIDE;

    QPixmap grabWindow(WId wid, int x, int y, int width, int height) const Q_DECL_OVERRIDE;

    EGLSurface primarySurface() const { return m_surface; }

    EGLDisplay display() const { return m_dpy; }

    void handleCursorMove(const QPoint &pos);

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
    virtual PowerState powerState() const;
    virtual void setPowerState(PowerState state);
#endif

private:
    void setPrimarySurface(EGLSurface surface);

    EGLDisplay m_dpy;
    QWindow *m_pointerWindow;
    EGLSurface m_surface;
    QPlatformCursor *m_cursor;

    friend class EglFSWindow;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSSCREEN_H
