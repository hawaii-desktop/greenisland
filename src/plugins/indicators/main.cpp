/****************************************************************************
 * This file is part of Indicators.
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

#include <QStringList>

#include <VIndicatorPlugin>

#include "datetime/datetimeindicator.h"
#include "power/powerindicator.h"

class GreenIslandIndicatorsPlugin : public VIndicatorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.maui.GreenIsland.VIndicatorFactoryInterface" FILE "indicators.json")
public:
    QStringList keys() const {
        return QStringList() << "datetime" << "power";
    }

    VIndicator *create(const QString &key) {
        if (key.toLower() == "datetime")
            return new DateTimeIndicator();
        if (key.toLower() == "power")
            return new PowerIndicator();
        return 0;
    }
};

#include "main.moc"
