/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

#include <KLocalizedString>
#include <KPackage/PackageTrader>

#include "compositorpackage.h"

#define DEFAULT_COMPOSITOR "org.hawaii.desktop"

CompositorPackage::CompositorPackage(QObject *, const QVariantList &)
{
}

void CompositorPackage::initPackage(KPackage::Package *package)
{
    package->setDefaultPackageRoot("plasma/greenisland/compositors/");

    // Main file
    package->addFileDefinition("main", "Compositor.qml",
                               i18n("Main compositor file"));
}

void CompositorPackage::pathChanged(KPackage::Package *package)
{
    if (!package->metadata().isValid())
        return;

    const QString pluginName = package->metadata().pluginName();

    if (!pluginName.isEmpty() && pluginName != DEFAULT_COMPOSITOR) {
        KPackage::Package pkg = KPackage::PackageTrader::self()->loadPackage("GreenIsland/Compositor");
        pkg.setPath(DEFAULT_COMPOSITOR);
        package->setFallbackPackage(pkg);
    } else if (package->fallbackPackage().isValid() && pluginName == DEFAULT_COMPOSITOR) {
        package->setFallbackPackage(KPackage::Package());
    }
}

K_EXPORT_PACKAGE_PACKAGE_WITH_JSON(CompositorPackage, "plasma-packagestructure-greenisland-compositor.json")

#include "compositorpackage.moc"
