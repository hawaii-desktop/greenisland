/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLANDCOMPOSITOR_P_H
#define GREENISLANDCOMPOSITOR_P_H

#include <QtCore/QProcess>
#include <QtCore/QTimer>

namespace GreenIsland {

class CompositorPrivate
{
    Q_DECLARE_PUBLIC(Compositor)
public:
    CompositorPrivate(Compositor *parent);
    ~CompositorPrivate();

    void closeShell();

    void dpms(bool on);

    void _q_shellStarted();
    void _q_shellFailed(QProcess::ProcessError error);
    void _q_shellReadyReadStandardOutput();
    void _q_shellReadyReadStandardError();
    void _q_shellAboutToClose();

    Compositor *q_ptr;

    Compositor::State state;
    QList<QWaylandSurface *> surfaces;
    QString shellFileName;
    QProcess *shellProcess;
    quint32 idleInterval;
    QTimer *idleTimer;
    int idleInhibit;
};

}

#endif // GREENISLANDCOMPOSITOR_P_H
