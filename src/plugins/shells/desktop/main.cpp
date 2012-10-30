/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <VShellPlugin>

#include "desktopshell.h"

class DesktopShellPlugin : public VShellPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.maui.GreenIsland.VShellFactoryInterface" FILE "desktop.json")
public:
    QStringList keys() const {
        return QStringList() << "desktop";
    }

    VShell *create(const QString &key, const QString &socketName) {
        if (key.toLower() == "desktop") {
            const char *name = socketName.isEmpty() ? 0 : socketName.toUtf8().constData();
            return new DesktopShell(name);
        }

        return 0;
    }
};

#include "main.moc"
