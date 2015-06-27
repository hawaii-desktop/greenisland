/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef SCREENBACKEND_H
#define SCREENBACKEND_H

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>

#include "output.h"

Q_DECLARE_LOGGING_CATEGORY(SCREEN_BACKEND)

namespace GreenIsland {

class ScreenBackend : public QObject
{
    Q_OBJECT
public:
    ScreenBackend(Compositor *compositor, QObject *parent = 0);
    virtual ~ScreenBackend();

    Compositor *compositor() const;
    QList<Output *> outputs() const;

public Q_SLOTS:
    virtual void acquireConfiguration() = 0;

Q_SIGNALS:
    void configurationAcquired();
    void outputAdded(Output *output);
    void outputRemoved(Output *output);
    void primaryOutputChanged(Output *output);

protected:
    Compositor *m_compositor;
    QList<Output *> m_outputs;
};

}

#endif // SCREENBACKEND_H
