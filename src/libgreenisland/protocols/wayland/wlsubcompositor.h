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

#ifndef WLSUBCOMPOSITOR_H
#define WLSUBCOMPOSITOR_H

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>
#include <QtCompositor/QWaylandGlobalInterface>
#include <QtCompositor/QWaylandSurface>
#include <QtCompositor/private/qwayland-server-wayland.h>

Q_DECLARE_LOGGING_CATEGORY(WLSUBCOMPOSITOR_TRACE)

namespace GreenIsland {

class WlSubSurface;

class WlSubCompositorGlobal : public QObject, public QWaylandGlobalInterface
{
public:
    explicit WlSubCompositorGlobal(QObject *parent = 0);

    const wl_interface *interface() const Q_DECL_OVERRIDE;
    void bind(wl_client *client, uint32_t version, uint32_t id) Q_DECL_OVERRIDE;
};

class WlSubCompositor : public QObject, public QtWaylandServer::wl_subcompositor
{
public:
    WlSubCompositor(wl_client *client, uint32_t name, uint32_t version, QObject *parent);
    ~WlSubCompositor();

    inline QList<WlSubSurface *> subSurfaces() const {
        return m_subSurfaces;
    }

protected:
    void subcompositor_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void subcompositor_get_subsurface(Resource *resource, uint32_t id,
                                      wl_resource *surfaceResource,
                                      wl_resource *parentResource) Q_DECL_OVERRIDE;

private:
    QList<WlSubSurface *> m_subSurfaces;

    WlSubSurface *toSubSurface(QWaylandSurface *surface);
    QWaylandSurface *mainSurface(QWaylandSurface *surface);

    friend class WlSubSurface;
};

}

#endif // WLSUBCOMPOSITOR_H
