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

#include "serverlogging_p.h"

Q_LOGGING_CATEGORY(gLcCore, "greenisland.compositor", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcGtkShell, "greenisland.protocols.gtkshell", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcGtkShellTrace, "greenisland.protocols.gtkshell.trace", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcScreencaster, "greenisland.protocols.screencaster", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcScreenshooter, "greenisland.protocols.screenshooter", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcTaskManager, "greenisland.protocols.taskmanager", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcXdgShell, "greenisland.protocols.xdgshell", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcXdgShellTrace, "greenisland.protocols.xdgshell.trace", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcScreenBackend, "greenisland.screenbackend", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcFakeScreenBackend, "greenisland.screenbackend.fake", QtDebugMsg)
Q_LOGGING_CATEGORY(gLcNativeScreenBackend, "greenisland.screenbackend.native", QtDebugMsg)
