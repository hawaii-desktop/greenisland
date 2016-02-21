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

#include <QtCore/qmath.h>
#include <QtCore/QRect>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/qpa/qplatformscreen.h>

#include "nativescreenbackend.h"
#include "screenbackend_p.h"
#include "serverlogging_p.h"

namespace GreenIsland {

namespace Server {

NativeScreenBackend::NativeScreenBackend(QObject *parent)
    : ScreenBackend(parent)
    , m_initialized(false)
{
}

void NativeScreenBackend::acquireConfiguration()
{
    Q_FOREACH (QScreen *qscreen, qGuiApp->screens())
        handleScreenAdded(qscreen);

    m_initialized = true;

    connect(qGuiApp, &QGuiApplication::screenAdded,
            this, &NativeScreenBackend::handleScreenAdded);
    connect(qGuiApp, &QGuiApplication::screenRemoved,
            this, &NativeScreenBackend::handleScreenRemoved);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this,
            [this](QScreen *qscreen) {
        Q_FOREACH (Screen *screen, screens()) {
            if (screen->screen() == qscreen) {
                Q_EMIT primaryScreenChanged(screen);
                break;
            }
        }
    });
#endif
}

void NativeScreenBackend::handleScreenAdded(QScreen *qscreen)
{
    qCDebug(gLcNativeScreenBackend) << "Screen added" << qscreen->name() << qscreen->availableGeometry();

    Screen *screen = new Screen(this);
    ScreenPrivate *screenPrivate = Screen::get(screen);
    screenPrivate->m_screen = qscreen;
    screenPrivate->m_manufacturer = QStringLiteral("Green Island");
    screenPrivate->m_model = qscreen->name();
    handleScreenChanged(qscreen, screen);

    ScreenBackend::get(this)->screens.append(screen);
    Q_EMIT screenAdded(screen);

    if (!m_initialized) {
        if (qGuiApp->primaryScreen() == qscreen)
            Q_EMIT primaryScreenChanged(screen);
    }

    connect(qscreen, &QScreen::availableGeometryChanged, this,
            [this, screen, qscreen](const QRect &) {
        handleScreenChanged(qscreen, screen);
    });
    connect(qscreen, &QScreen::physicalSizeChanged, this,
            [this, screen, qscreen](const QSizeF &) {
        handleScreenChanged(qscreen, screen);
    });
    connect(qscreen, &QScreen::primaryOrientationChanged, this,
            [this, screen, qscreen](Qt::ScreenOrientation) {
        handleScreenChanged(qscreen, screen);
    });
}

void NativeScreenBackend::handleScreenRemoved(QScreen *qscreen)
{
    qCDebug(gLcNativeScreenBackend) << "Screen removed" << qscreen->name() << qscreen->availableGeometry();

    QList<Screen *> list = ScreenBackend::get(this)->screens;
    auto it = list.begin();
    while (it != list.end()) {
        Screen *screen = (*it);
        if (screen->screen() == qscreen) {
            it = list.erase(it);
            Q_EMIT screenRemoved(screen);
            screen->deleteLater();
        }
    }
}

void NativeScreenBackend::handleScreenChanged(QScreen *qscreen, Screen *screen)
{
    qCDebug(gLcNativeScreenBackend) << "Screen" << qscreen->name() << "have been changed";

    ScreenPrivate *screenPrivate = Screen::get(screen);

    screenPrivate->setPosition(qscreen->availableGeometry().topLeft());
    screenPrivate->setSize(qscreen->availableGeometry().size());
    screenPrivate->setRefreshRate(qscreen->refreshRate() * 1000);
    screenPrivate->setPhysicalSize(qscreen->physicalSize());
    screenPrivate->setScaleFactor(qFloor(qscreen->devicePixelRatio()));

    switch (qscreen->orientation()) {
    case Qt::PortraitOrientation:
        screenPrivate->setTransform(Screen::Transform90);
        break;
    case Qt::InvertedLandscapeOrientation:
        screenPrivate->setTransform(Screen::Transform180);
        break;
    case Qt::InvertedPortraitOrientation:
        screenPrivate->setTransform(Screen::Transform270);
        break;
    default:
        break;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    QPlatformScreen::SubpixelAntialiasingType subpixel = qscreen->handle()->subpixelAntialiasingTypeHint();
    switch (subpixel) {
    case QPlatformScreen::Subpixel_None:
        screenPrivate->setSubpixel(Screen::SubpixelNone);
        break;
    case QPlatformScreen::Subpixel_RGB:
        screenPrivate->setSubpixel(Screen::SubpixelHorizontalRgb);
        break;
    case QPlatformScreen::Subpixel_BGR:
        screenPrivate->setSubpixel(Screen::SubpixelHorizontalBgr);
        break;
    case QPlatformScreen::Subpixel_VRGB:
        screenPrivate->setSubpixel(Screen::SubpixelVerticalRgb);
        break;
    case QPlatformScreen::Subpixel_VBGR:
        screenPrivate->setSubpixel(Screen::SubpixelVerticalBgr);
        break;
    default:
        break;
    }
#endif
}

} // namespace Server

} // namespace GreenIsland

#include "moc_nativescreenbackend.cpp"
