/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef SYSTEMCLIENT_H
#define SYSTEMCLIENT_H

#include <QPair>

#include <QtCompositor/private/qwloutput_p.h>
#include <QtCompositor/QWaylandSurface>

typedef QList<QWaylandSurface *> WaylandSurfaceList;
typedef QPair<QtWayland::Output *, QWaylandSurface *> OutputSurfaceMapping;

class SystemClient
{
public:
    SystemClient(wl_client *client);

    wl_client *client() const;

    wl_resource *clientResource() const;
    void setClientResource(wl_resource *res);
    void unbindClientResource();

    QWaylandSurface *surfaceForOutput(QtWayland::Output *output) const;
    void mapSurfaceToOutput(QWaylandSurface *surface, QtWayland::Output *output);

    WaylandSurfaceList surfaces() const;

    bool isReadySent() const;
    void setReady();

private:
    struct wl_client *m_client;
    struct wl_resource *m_resource;
    QList<OutputSurfaceMapping> m_mappings;
    bool m_readySent;
};

typedef QList<SystemClient *> SystemClientList;

#endif // SYSTEMCLIENT_H
