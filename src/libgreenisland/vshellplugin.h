/****************************************************************************
 * This file is part of libGreenIsland.
 *
 * Copyright (c) Pier Luigi Fiorini
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

#ifndef VSHELLPLUGIN_H
#define VSHELLPLUGIN_H

#include <QtCore/qplugin.h>

#include <VGreenIslandGlobal>
#include <VShell>

struct GREENISLAND_EXPORT VShellFactoryInterface {
    virtual QStringList keys() const = 0;
    virtual VShell *create(const QString &key) = 0;
};

#define VShellFactoryInterface_iid "org.maui.GreenIsland.VShellFactoryInterface"

Q_DECLARE_INTERFACE(VShellFactoryInterface, VShellFactoryInterface_iid)

class GREENISLAND_EXPORT VShellPlugin : public QObject, public VShellFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(VShellFactoryInterface)
public:
    explicit VShellPlugin(QObject *parent = 0);

    virtual ~VShellPlugin();

    /*!
        Returns a list of keys this plugins supports.

        \sa create();
    */
    virtual QStringList keys() const = 0;

    /*!
        Creates and returns a VShell object for the given key.
        If a plugin cannot create the indicator, it should return 0 instead.

        \param key the key.

        \sa keys()
    */
    virtual VShell *create(const QString &key) = 0;
};

#endif // VSHELLPLUGIN_H
