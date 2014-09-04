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

#include "compositor.h"
#include "output.h"
#include "outputwindow.h"

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
    KScreen::Output *output;

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

    // Refresh rate
    q->setRefreshRate(qFloor(output->currentMode()->refreshRate() * 1000));

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
    q->window()->resize(output->currentMode()->size());
    q->window()->setMinimumSize(output->currentMode()->size());
    q->window()->setMaximumSize(output->currentMode()->size());

    // Set geometry
    q->setGeometry(QRect(q->geometry().topLeft(),
                         output->currentMode()->size()));
}

void OutputPrivate::_q_posChanged()
{
    Q_Q(Output);

    // Move window
    q->window()->setPosition(output->pos());

    // Set geometry
    q->setGeometry(QRect(output->pos(), q->geometry().size()));
}

/*
 * Output
 */

Output::Output(Compositor *compositor, KScreen::Output *output)
    : QWaylandOutput(compositor, new OutputWindow(compositor))
    , d_ptr(new OutputPrivate(this))
{
    Q_D(Output);

    // Save compositor and output instances
    d->compositor = compositor;
    d->output = output;

    // Set output properties
    setPhysicalSize(d->output->sizeMm());
    d->_q_currentModeIdChanged();
    d->_q_posChanged();

    // React to output changes
    connect(output, &KScreen::Output::isPrimaryChanged,
            this, &Output::primaryChanged,
            Qt::UniqueConnection);
    connect(output, SIGNAL(currentModeIdChanged()),
            this, SLOT(_q_currentModeIdChanged()),
            Qt::UniqueConnection);
    connect(output, SIGNAL(posChanged()),
            this, SLOT(_q_posChanged()),
            Qt::UniqueConnection);

    // Show window
    OutputWindow *outputWindow = qobject_cast<OutputWindow *>(window());
    if (outputWindow)
        outputWindow->setOutput(this);
}

Compositor *Output::compositor() const
{
    Q_D(const Output);
    return d->compositor;
}

KScreen::Output *Output::output() const
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
    return d->output->isPrimary();
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

#include "moc_output.cpp"
