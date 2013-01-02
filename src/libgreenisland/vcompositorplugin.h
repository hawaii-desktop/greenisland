/****************************************************************************
 * This file is part of libGreenIsland.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef VCOMPOSITORPLUGIN_H
#define VCOMPOSITORPLUGIN_H

#include <QtCore/qplugin.h>

#include <VGreenIslandGlobal>
#include <VCompositor>

struct GREENISLAND_EXPORT VCompositorFactoryInterface {
    virtual QStringList keys() const = 0;
    virtual VCompositor *create(const QString &key) = 0;
};

#define VCompositorFactoryInterface_iid "org.hawaii.GreenIsland.VCompositorFactoryInterface"

Q_DECLARE_INTERFACE(VCompositorFactoryInterface, VCompositorFactoryInterface_iid)

class GREENISLAND_EXPORT VCompositorPlugin : public QObject, public VCompositorFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(VCompositorFactoryInterface)
public:
    explicit VCompositorPlugin(QObject *parent = 0);

    virtual ~VCompositorPlugin();

    /*!
        Returns a list of keys this plugins supports.

        \sa create();
    */
    virtual QStringList keys() const = 0;

    /*!
        Creates and returns a VCompositor object for the given key.
        If a plugin cannot create the indicator, it should return 0 instead.

        \param key the key.

        \sa keys()
    */
    virtual VCompositor *create(const QString &key) = 0;
};

#endif // VCOMPOSITORPLUGIN_H
