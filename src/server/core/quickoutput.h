/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2016 Pier Luigi Fiorini
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

#ifndef GREENISLAND_QUICKOUTPUT_H
#define GREENISLAND_QUICKOUTPUT_H

#include <GreenIsland/QtWaylandCompositor/QWaylandQuickOutput>
#include <GreenIsland/Server/Screen>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class Screen;
class QuickOutputPrivate;

class GREENISLANDSERVER_EXPORT QuickOutput : public QWaylandQuickOutput
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuickOutput)
    Q_PROPERTY(Screen *nativeScreen READ nativeScreen WRITE setNativeScreen NOTIFY nativeScreenChanged DESIGNABLE false)
    Q_PROPERTY(PowerState powerState READ powerState WRITE setPowerState NOTIFY powerStateChanged)
    Q_PROPERTY(QSize hotSpotSize READ hotSpotSize WRITE setHotSpotSize NOTIFY hotSpotSizeChanged)
    Q_PROPERTY(quint64 hotSpotThreshold READ hotSpotThreshold WRITE setHotSpotThreshold NOTIFY hotSpotThresholdChanged)
    Q_PROPERTY(quint64 hotSpotPushTime READ hotSpotPushTime WRITE setHotSpotPushTime NOTIFY hotSpotPushTimeChanged)
public:
    enum PowerState {
        PowerStateOn,
        PowerStateStandby,
        PowerStateSuspend,
        PowerStateOff
    };
    Q_ENUM(PowerState)

    enum HotSpot {
        TopLeftHotSpot = 0,
        TopRightHotSpot,
        BottomLeftHotSpot,
        BottomRightHotSpot
    };
    Q_ENUM(HotSpot)

    QuickOutput();
    QuickOutput(QWaylandCompositor *compositor);

    Screen *nativeScreen() const;
    void setNativeScreen(Screen *screen);

    PowerState powerState() const;
    void setPowerState(PowerState state);

    QSize hotSpotSize() const;
    void setHotSpotSize(const QSize &size);

    quint64 hotSpotThreshold() const;
    void setHotSpotThreshold(quint64 value);

    quint64 hotSpotPushTime() const;
    void setHotSpotPushTime(quint64 value);

protected:
    void initialize() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void nativeScreenChanged();
    void powerStateChanged();
    void hotSpotSizeChanged();
    void hotSpotThresholdChanged();
    void hotSpotPushTimeChanged();
    void hotSpotTriggered(HotSpot hotSpot);

private:
    QuickOutputPrivate *const d_ptr;

private Q_SLOTS:
    void readContent();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_QUICKOUTPUT_H
