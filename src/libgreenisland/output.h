/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef GREENISLAND_OUTPUT_H
#define GREENISLAND_OUTPUT_H

#include <QtCompositor/QWaylandQuickOutput>

#include <KScreen/Output>

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class Compositor;
class OutputPrivate;
class ScreenManagerPrivate;

class GREENISLAND_EXPORT Output : public QWaylandQuickOutput
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int number READ number CONSTANT)
    Q_PROPERTY(bool primary READ isPrimary NOTIFY primaryChanged)
    Q_PROPERTY(QSize hotSpotSize READ hotSpotSize WRITE setHotSpotSize NOTIFY hotSpotSizeChanged)
    Q_PROPERTY(quint64 hotSpotThreshold READ hotSpotThreshold WRITE setHotSpotThreshold NOTIFY hotSpotThresholdChanged)
    Q_PROPERTY(quint64 hotSpotPushTime READ hotSpotPushTime WRITE setHotSpotPushTime NOTIFY hotSpotPushTimeChanged)
    Q_ENUMS(HotSpot)
public:
    enum HotSpot {
        TopLeftHotSpot = 0,
        TopRightHotSpot,
        BottomLeftHotSpot,
        BottomRightHotSpot
    };

    Output(Compositor *compositor, const KScreen::OutputPtr &output);

    Compositor *compositor() const;
    KScreen::OutputPtr output() const;

    QString name() const;

    int number() const;

    bool isPrimary() const;

    QSize hotSpotSize() const;
    void setHotSpotSize(const QSize &size);

    quint64 hotSpotThreshold() const;
    void setHotSpotThreshold(quint64 value);

    quint64 hotSpotPushTime() const;
    void setHotSpotPushTime(quint64 value);

    void loadScene();

    // Maps global coordinates to local space
    Q_INVOKABLE QPointF mapToOutput(const QPointF &pt);

    // Map output local coordinates to global space
    Q_INVOKABLE QPointF mapToGlobal(const QPointF &pt);

Q_SIGNALS:
    void primaryChanged();
    void hotSpotSizeChanged();
    void hotSpotThresholdChanged();
    void hotSpotPushTimeChanged();
    void hotSpotTriggered(HotSpot hotSpot);

private:
    Q_DECLARE_PRIVATE(Output)
    OutputPrivate *const d_ptr;

    friend class ScreenManagerPrivate;

    void setPrimary(bool value);

    Q_PRIVATE_SLOT(d_func(), void _q_currentModeIdChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_posChanged())
};

}

#endif // GREENISLAND_OUTPUT_H
