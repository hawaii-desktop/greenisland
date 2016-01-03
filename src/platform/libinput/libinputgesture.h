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

#ifndef GREENISLAND_LIBINPUTGESTURE_H
#define GREENISLAND_LIBINPUTGESTURE_H

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

struct libinput_event_gesture;

namespace GreenIsland {

namespace Platform {

class LibInputHandler;

class LibInputGesture
{
public:
    LibInputGesture(LibInputHandler *handler);

    void handlePinchBegin(libinput_event_gesture *event);
    void handlePinchEnd(libinput_event_gesture *event);
    void handlePinchUpdate(libinput_event_gesture *event);

    void handleSwipeBegin(libinput_event_gesture *event);
    void handleSwipeEnd(libinput_event_gesture *event);
    void handleSwipeUpdate(libinput_event_gesture *event);

private:
    LibInputHandler *m_handler;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_LIBINPUTGESTURE_H
