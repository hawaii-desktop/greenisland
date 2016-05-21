/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef GREENISLAND_LIBINPUTTOUCH_H
#define GREENISLAND_LIBINPUTTOUCH_H

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

struct libinput_device;
struct libinput_event_touch;

class QTouchDevice;

namespace GreenIsland {

namespace Platform {

class LibInputHandler;
class LibInputTouchPrivate;

class LibInputTouch
{
    Q_DECLARE_PRIVATE(LibInputTouch)
public:
    LibInputTouch(LibInputHandler *handler);
    ~LibInputTouch();

    QTouchDevice *registerDevice(libinput_device *device);
    void unregisterDevice(libinput_device *device, QTouchDevice **td);

    void handleTouchUp(libinput_event_touch *event);
    void handleTouchDown(libinput_event_touch *event);
    void handleTouchMotion(libinput_event_touch *event);
    void handleTouchCancel(libinput_event_touch *event);
    void handleTouchFrame(libinput_event_touch *event);

private:
    LibInputTouchPrivate *const d_ptr;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_LIBINPUTTOUCH_H
