/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
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

#include <VShellPlugin>

#include "desktopshell.h"

#if 0
class DesktopShellPlugin : public VShellPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.maui.GreenIsland.VShellFactoryInterface" FILE "desktop.json")
public:
    QStringList keys() const {
        return QStringList() << "desktop";
    }

    VShell *create(const QString &key) {
        if (key.toLower() == "desktop")
            return new DesktopShell();

        return 0;
    }
};
#else
#include <QGuiApplication>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    new DesktopShell();

    return app.exec();
}
#endif

#include "main.moc"
