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

class GREENISLAND_EXPORT Output : public QWaylandQuickOutput
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int number READ number CONSTANT)
    Q_PROPERTY(bool primary READ isPrimary NOTIFY primaryChanged)
public:
    Output(Compositor *compositor, KScreen::Output *output);
    Output(Compositor *compositor, const KScreen::OutputPtr &output);

    Compositor *compositor() const;

    KScreen::OutputPtr output() const;

    QString name() const;

    int number() const;

    bool isPrimary() const;

    // Maps global coordinates to local space
    Q_INVOKABLE QPointF mapToOutput(const QPointF &pt);

    // Map output local coordinates to global space
    Q_INVOKABLE QPointF mapToGlobal(const QPointF &pt);

Q_SIGNALS:
    void primaryChanged();

private:
    Q_DECLARE_PRIVATE(Output)
    OutputPrivate *const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_currentModeIdChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_posChanged())
};

}

#endif // GREENISLAND_OUTPUT_H
