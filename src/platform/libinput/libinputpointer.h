/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLAND_LIBINPUTPOINTER_H
#define GREENISLAND_LIBINPUTPOINTER_H

#include <QtCore/QPoint>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

struct libinput_event_pointer;

namespace GreenIsland {

namespace Platform {

class LibInputHandler;

class LibInputPointer
{
public:
    LibInputPointer(LibInputHandler *handler);

    void setPosition(const QPoint &pos);

    void handleButton(libinput_event_pointer *e);
    void handleMotion(libinput_event_pointer *e);
    void handleAbsoluteMotion(libinput_event_pointer *e);
    void handleAxis(libinput_event_pointer *e);

private:
    LibInputHandler *m_handler;
    QPoint m_pt;
    Qt::MouseButtons m_buttons;

    void processMotion(const QPoint &pos);
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_LIBINPUTPOINTER_H
