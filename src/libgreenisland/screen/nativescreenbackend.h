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

#ifndef NATIVESCREENBACKEND_H
#define NATIVESCREENBACKEND_H

#include "screenbackend.h"

Q_DECLARE_LOGGING_CATEGORY(NATIVE_BACKEND)

namespace GreenIsland {

class NativeScreenBackend : public ScreenBackend
{
    Q_OBJECT
public:
    NativeScreenBackend(Compositor *compositor, QObject *parent = 0);
    ~NativeScreenBackend();

public Q_SLOTS:
    void acquireConfiguration();

private:
    QMap<QScreen *, Output *> m_screenMap;

private Q_SLOTS:
    void screenAdded(QScreen *screen);
    void screenRemoved(QScreen *screen);

    void changePosition(QScreen *screen);
    void changePhysicalSize(QScreen *screen);
    void changeOrientation(QScreen *screen);
};

}

#endif // NATIVESCREENBACKEND_H
