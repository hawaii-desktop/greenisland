/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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
    Q_PROPERTY(QSize hotSpotSize READ hotSpotSize WRITE setHotSpotSize NOTIFY hotSpotSizeChanged)
    Q_PROPERTY(quint64 hotSpotThreshold READ hotSpotThreshold WRITE setHotSpotThreshold NOTIFY hotSpotThresholdChanged)
    Q_PROPERTY(quint64 hotSpotPushTime READ hotSpotPushTime WRITE setHotSpotPushTime NOTIFY hotSpotPushTimeChanged)
public:
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
