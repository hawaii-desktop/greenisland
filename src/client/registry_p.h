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

#ifndef GREENISLANDCLIENT_REGISTRY_P_H
#define GREENISLANDCLIENT_REGISTRY_P_H

#include <QtCore/QLoggingCategory>
#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Client/Registry>
#include <GreenIsland/client/private/qwayland-wayland.h>

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

Q_DECLARE_LOGGING_CATEGORY(WLREGISTRY)

namespace GreenIsland {

namespace Client {

class GREENISLANDCLIENT_EXPORT RegistryPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(Registry)
public:
    RegistryPrivate();
    ~RegistryPrivate();

    void setup();

    template <typename T>
    T *bind(Registry::Interface interface);

    wl_display *display;
    wl_registry *registry;
    wl_callback *callback;

    static RegistryPrivate *get(Registry *r) { return r->d_func(); }

private:
    struct InterfaceInfo {
        Registry::Interface interface;
        quint32 name;
        quint32 version;
    };
    QList<InterfaceInfo> interfaces;

    static const struct wl_registry_listener s_registryListener;
    static const struct wl_callback_listener s_callbackListener;

    void handleAnnounce(const char *interface, quint32 name, quint32 version);
    void handleRemove(quint32 name);
    void handleSync();

    static void globalAnnounce(void *data, wl_registry *registry, uint32_t name,
                               const char *interface, uint32_t version);
    static void globalRemove(void *data, wl_registry *registry, uint32_t name);
    static void globalSync(void *data, wl_callback *callback, uint32_t serial);

};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_REGISTRY_P_H

