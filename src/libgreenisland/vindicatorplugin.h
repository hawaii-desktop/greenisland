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

#ifndef VINDICATORPLUGIN_H
#define VINDICATORPLUGIN_H

#include <QtCore/qplugin.h>

#include <VGreenIslandGlobal>

class VIndicator;

struct GREENISLAND_EXPORT VIndicatorFactoryInterface {
    virtual VIndicator *create(const QString &mimeType) = 0;
    virtual QStringList keys() const = 0;
};

#define VIndicatorFactoryInterface_iid "org.maui.GreenIsland.VIndicatorFactoryInterface"

Q_DECLARE_INTERFACE(VIndicatorFactoryInterface, VIndicatorFactoryInterface_iid)

class GREENISLAND_EXPORT VIndicatorPlugin : public QObject, public VIndicatorFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(VIndicatorFactoryInterface)
public:
    explicit VIndicatorPlugin(QObject *parent = 0);

    virtual ~VIndicatorPlugin();

    /*!
        Returns a list of keys this plugins supports.

        \sa create();
    */
    virtual QStringList keys() const = 0;

    /*!
        Creates and returns a VIndicator object for the given key.
        If a plugin cannot create the indicator, it should return 0 instead.

        \param key the key.

        \sa keys()
    */
    virtual VIndicator *create(const QString &key) = 0;
};

#endif // VINDICATORPLUGIN_H
