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

#include <QtCore/QtMath>

#include <KScreen/EDID>

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

    void _q_currentModeIdChanged();
    void _q_posChanged();

    Compositor *compositor;
    KScreen::OutputPtr output;

private:
    Q_DECLARE_PUBLIC(Output)
    Output *const q_ptr;
};

OutputPrivate::OutputPrivate(Output *parent)
    : compositor(Q_NULLPTR)
    , output(Q_NULLPTR)
    , q_ptr(parent)
{
}

void OutputPrivate::_q_currentModeIdChanged()
{
    Q_Q(Output);

    // Mode
    QWaylandOutput::Mode mode;
    mode.size = output->currentMode()->size();
    mode.refreshRate = output->currentMode()->refreshRate();
    q->setMode(mode);

    // Rotation
    switch (output->rotation()) {
    case KScreen::Output::None:
        q->setTransform(QWaylandOutput::TransformNormal);
        break;
    case KScreen::Output::Left:
        q->setTransform(QWaylandOutput::Transform90);
        break;
    case KScreen::Output::Inverted:
        q->setTransform(QWaylandOutput::Transform180);
        break;
    case KScreen::Output::Right:
        q->setTransform(QWaylandOutput::Transform270);
        break;
    }

    // Resize window
    q->window()->resize(mode.size);
    q->window()->setMinimumSize(mode.size);
    q->window()->setMaximumSize(mode.size);
}

void OutputPrivate::_q_posChanged()
{
    Q_Q(Output);

    // Move window
    q->window()->setPosition(output->pos());

    // Set position
    q->setPosition(output->pos());
}

/*
 * Output
 */

Output::Output(Compositor *compositor, const KScreen::OutputPtr &output)
    : QWaylandQuickOutput(compositor, new OutputWindow(compositor),
                          output->edid()->vendor(), output->edid()->serial())
    , d_ptr(new OutputPrivate(this))
{
    Q_D(Output);
    d->compositor = compositor;
    d->output = output;

    // Set output properties
    setPhysicalSize(d->output->sizeMm());
    d->_q_currentModeIdChanged();
    d->_q_posChanged();

    // React to output changes
    connect(output.data(), SIGNAL(currentModeIdChanged()),
            this, SLOT(_q_currentModeIdChanged()),
            Qt::UniqueConnection);
    connect(output.data(), SIGNAL(posChanged()),
            this, SLOT(_q_posChanged()),
            Qt::UniqueConnection);

    // Show window
    OutputWindow *outputWindow = qobject_cast<OutputWindow *>(quickWindow());
    if (outputWindow)
        outputWindow->setOutput(this);
}

Compositor *Output::compositor() const
{
    Q_D(const Output);
    return d->compositor;
}

KScreen::OutputPtr Output::output() const
{
    Q_D(const Output);
    return d->output;
}

QString Output::name() const
{
    Q_D(const Output);
    return d->output->name();
}

int Output::number() const
{
    Q_D(const Output);
    return d->output->id();
}

bool Output::isPrimary() const
{
    Q_D(const Output);
    return d->compositor->primaryOutput() == this;
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
