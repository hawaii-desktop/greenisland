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

#ifndef GREENISLAND_OUTPUT_H
#define GREENISLAND_OUTPUT_H

#include <QtCompositor/QWaylandQuickOutput>
#include <QtCompositor/QtCompositorVersion>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

class Compositor;
class FakeScreenBackend;
class OutputPrivate;
class OutputWindow;
class NativeScreenBackend;

class GREENISLANDSERVER_EXPORT Output : public QWaylandQuickOutput
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

//#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 6, 0)
#if 0
    Output(Compositor *compositor, const QString &name,
           const QString &manufacturer, const QString &model,
           const QWaylandOutputModeList &modes);
#else
    Output(Compositor *compositor, const QString &name,
           const QString &manufacturer, const QString &model);
#endif

    Compositor *compositor() const;
    OutputWindow *outputWindow() const;

    QString name() const;

    int number() const;

    bool isPrimary() const;

    QSize hotSpotSize() const;
    void setHotSpotSize(const QSize &size);

    quint64 hotSpotThreshold() const;
    void setHotSpotThreshold(quint64 value);

    quint64 hotSpotPushTime() const;
    void setHotSpotPushTime(quint64 value);

    // Maps global coordinates to local space
    Q_INVOKABLE QPointF mapToOutput(const QPointF &pt);

    // Map output local coordinates to global space
    Q_INVOKABLE QPointF mapToGlobal(const QPointF &pt);

public Q_SLOTS:
    void loadScene();

Q_SIGNALS:
    void primaryChanged();
    void hotSpotSizeChanged();
    void hotSpotThresholdChanged();
    void hotSpotPushTimeChanged();
    void hotSpotTriggered(HotSpot hotSpot);

private:
    Q_DECLARE_PRIVATE(Output)
    OutputPrivate *const d_ptr;

    friend class FakeScreenBackend;
    friend class NativeScreenBackend;

    void setPrimary(bool value);
};

}

#endif // GREENISLAND_OUTPUT_H