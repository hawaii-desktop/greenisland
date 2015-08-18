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

#include "compositor.h"
#include "output.h"
#include "outputwindow.h"

namespace GreenIsland {

/*
 * OutputPrivate
 */

class OutputPrivate
{
public:
    OutputPrivate(Output *parent);

    Compositor *compositor;

    int id;
    QString name;
    bool primary;

    QSize hotSpotSize;
    quint64 hotSpotThreshold;
    quint64 hotSpotPushTime;

private:
    Q_DECLARE_PUBLIC(Output)
    Output *const q_ptr;
};

OutputPrivate::OutputPrivate(Output *parent)
    : primary(false)
    , hotSpotSize(QSize(10, 10))
    , hotSpotThreshold(1000)
    , hotSpotPushTime(150)
    , q_ptr(parent)
{
    static int seed = 0;
    id = seed++;
}

/*
 * Output
 */

//#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 6, 0)
#if 0
Output::Output(Compositor *compositor, const QString &name,
               const QString &manufacturer, const QString &model,
               const QWaylandOutputModeList &modes)
    : QWaylandQuickOutput(compositor, new OutputWindow(this),
                          manufacturer, model, modes)
#else
Output::Output(Compositor *compositor, const QString &name,
               const QString &manufacturer, const QString &model)
    : QuickOutput(compositor, new OutputWindow(this),
                          manufacturer, model)
#endif
    , d_ptr(new OutputPrivate(this))
{
    Q_D(Output);
    d->compositor = compositor;
    d->name = name;
}

Compositor *Output::compositor() const
{
    Q_D(const Output);
    return d->compositor;
}

OutputWindow *Output::outputWindow() const
{
    return static_cast<OutputWindow *>(window());
}

QString Output::name() const
{
    Q_D(const Output);
    return d->name;
}

int Output::number() const
{
    Q_D(const Output);
    return d->id;
}

bool Output::isPrimary() const
{
    Q_D(const Output);
    return d->primary;
}

void Output::setPrimary(bool value)
{
    Q_D(Output);

    if (d->primary == value)
        return;

    d->primary = value;
    Q_EMIT primaryChanged();
}

QSize Output::hotSpotSize() const
{
    Q_D(const Output);
    return d->hotSpotSize;
}

void Output::setHotSpotSize(const QSize &size)
{
    Q_D(Output);

    if (d->hotSpotSize == size)
        return;

    d->hotSpotSize = size;
    Q_EMIT hotSpotSizeChanged();
}

quint64 Output::hotSpotThreshold() const
{
    Q_D(const Output);
    return d->hotSpotThreshold;
}

void Output::setHotSpotThreshold(quint64 value)
{
    Q_D(Output);

    if (d->hotSpotThreshold == value)
        return;

    d->hotSpotThreshold = value;
    Q_EMIT hotSpotThresholdChanged();
}

quint64 Output::hotSpotPushTime() const
{
    Q_D(const Output);
    return d->hotSpotPushTime;
}

void Output::setHotSpotPushTime(quint64 value)
{
    Q_D(Output);

    if (d->hotSpotThreshold == value)
        return;

    d->hotSpotPushTime = value;
    Q_EMIT hotSpotPushTimeChanged();
}

void Output::loadScene()
{
    // Show output window and load scene
    static_cast<OutputWindow *>(window())->loadScene();
}

QPointF Output::mapToOutput(const QPointF &pt)
{
    QPointF pos(geometry().topLeft());
    return QPointF(pt.x() - pos.x(), pt.y() - pos.y());
}

QPointF Output::mapToGlobal(const QPointF &pt)
{
    return geometry().topLeft() + pt;
}

}

#include "moc_output.cpp"
