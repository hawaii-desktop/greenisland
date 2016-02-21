/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef GREENISLAND_LIBINPUTHANDLER_P_H
#define GREENISLAND_LIBINPUTHANDLER_P_H

#include <QtCore/private/qobject_p.h>

#include "libinput/libinputgesture.h"
#include "libinput/libinputkeyboard.h"
#include "libinput/libinputpointer.h"
#include "libinput/libinputtouch.h"
#include "udev/udev.h"

#include <libinput.h>

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

namespace GreenIsland {

namespace Platform {

class LibInputHandlerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(LibInputHandler)
public:
    LibInputHandlerPrivate();
    ~LibInputHandlerPrivate();

    void setup();
    void initialize();
    void deactivate();

    void _q_liEventHandler();

    static void logHandler(libinput *handle, libinput_log_priority priority,
                           const char *format, va_list args);

    static int restrictedOpenCallback(const char *path, int flags, void *user_data);
    static void restrictedCloseCallback(int fd, void *user_data);

    bool initialized;

    Udev *udev;
    libinput *li;

    LibInputKeyboard *keyboard;
    int keyboardCount;
    bool keyboardSuspended;

    LibInputPointer *pointer;
    int pointerCount;
    bool pointerSuspended;

    LibInputTouch *touch;
    int touchCount;
    bool touchSuspended;

    LibInputGesture *gesture;
    int gestureCount;

    bool suspended;

    static const struct libinput_interface liInterface;

private:
    int restrictedOpen(const char *path, int flags);
    void restrictedClose(int fd);
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_LIBINPUTHANDLER_P_H
