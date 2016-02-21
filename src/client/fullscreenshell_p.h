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

#ifndef GREENISLAND_FULLSCREENSHELL_P_H
#define GREENISLAND_FULLSCREENSHELL_P_H

#include <QtCore/QLoggingCategory>
#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Client/FullScreenShell>
#include <GreenIsland/client/private/qwayland-fullscreen-shell.h>

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

Q_DECLARE_LOGGING_CATEGORY(FSH_CLIENT_PROTOCOL)

namespace GreenIsland {

namespace Client {

class GREENISLANDCLIENT_EXPORT FullScreenShellPrivate
        : public QObjectPrivate
        , public QtWayland::_wl_fullscreen_shell
{
    Q_DECLARE_PUBLIC(FullScreenShell)
public:
    FullScreenShellPrivate();

    FullScreenShell::Capabilities capabilities;

    static FullScreenShellPrivate *get(FullScreenShell *fsh) { return fsh->d_func(); }

protected:
    void fullscreen_shell_capability(uint32_t capabilty) Q_DECL_OVERRIDE;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLAND_FULLSCREENSHELL_P_H
