/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *    Giulio Camuffo
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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

#ifndef GREENISLAND_GLOBALINTERFACE_H
#define GREENISLAND_GLOBALINTERFACE_H

#include <QtCore/qglobal.h>

#include <greenisland/greenisland_export.h>

struct wl_interface;
struct wl_client;

namespace GreenIsland {

class WlCompositor;

class GREENISLAND_EXPORT GlobalInterface
{
public:
    GlobalInterface();
    virtual ~GlobalInterface();

    virtual const wl_interface *interface() const = 0;
    virtual quint32 version() const;

protected:
    virtual void bind(wl_client *client, quint32 version, quint32 id) = 0;

    friend class GreenIsland::WlCompositor;
};

} // namespace GreenIsland

#endif // GREENISLAND_GLOBALINTERFACE_H
