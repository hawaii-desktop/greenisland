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

#ifndef GREENISLAND_SCREENBACKEND_H
#define GREENISLAND_SCREENBACKEND_H

#include <QtCore/QObject>

#include <GreenIsland/server/greenislandserver_export.h>

class QScreen;

namespace GreenIsland {

namespace Server {

class ScreenPrivate;
class ScreenBackendPrivate;

class GREENISLANDSERVER_EXPORT Screen : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Screen)
    Q_PROPERTY(QScreen *screen READ screen CONSTANT)
    Q_PROPERTY(QString manufacturer READ manufacturer CONSTANT)
    Q_PROPERTY(QString model READ model CONSTANT)
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(int refreshRate READ refreshRate NOTIFY refreshRateChanged)
    Q_PROPERTY(QSizeF physicalSize READ physicalSize NOTIFY physicalSizeChanged)
    Q_PROPERTY(Subpixel subpixel READ subpixel NOTIFY subpixelChanged)
    Q_PROPERTY(Transform transform READ transform NOTIFY transformChanged)
    Q_PROPERTY(int scaleFactor READ scaleFactor NOTIFY scaleFactorChanged)
public:
    enum Subpixel {
      SubpixelUnknown = 0,
      SubpixelNone,
      SubpixelHorizontalRgb,
      SubpixelHorizontalBgr,
      SubpixelVerticalRgb,
      SubpixelVerticalBgr
    };
    Q_ENUM(Subpixel)

    enum Transform {
        TransformNormal = 0,
        Transform90,
        Transform180,
        Transform270,
        TransformFlipped,
        TransformFlipped90,
        TransformFlipped180,
        TransformFlipped270
    };
    Q_ENUM(Transform)

    Screen(QObject *parent = Q_NULLPTR);

    QScreen *screen() const;

    QString manufacturer() const;
    QString model() const;

    QPoint position() const;
    QSize size() const;
    int refreshRate() const;
    QSizeF physicalSize() const;
    Subpixel subpixel() const;
    Transform transform() const;
    int scaleFactor() const;

    static ScreenPrivate *get(Screen *screen) { return screen->d_func(); }

Q_SIGNALS:
    void positionChanged();
    void sizeChanged();
    void refreshRateChanged();
    void physicalSizeChanged();
    void subpixelChanged();
    void transformChanged();
    void scaleFactorChanged();
};

class GREENISLANDSERVER_EXPORT ScreenBackend : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ScreenBackend)
public:
    ScreenBackend(QObject *parent = Q_NULLPTR);
    virtual ~ScreenBackend();

    QList<Screen *> screens() const;

    virtual void acquireConfiguration() = 0;

    static ScreenBackendPrivate *get(ScreenBackend *backend) { return backend->d_func(); }

Q_SIGNALS:
    void screenAdded(Screen *screen);
    void screenRemoved(Screen *screen);
    void primaryScreenChanged(Screen *screen);
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_SCREENBACKEND_H
