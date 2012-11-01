/****************************************************************************
 * This file is part of Green Island.
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

#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <QStringList>

#include <VShellPlugin>

#include "cmakedirs.h"

class GreenIsland : public QApplication
{
    Q_OBJECT
public:
    GreenIsland(int &argc, char **argv)
        : QApplication(argc, argv) {
        // Set application information
        setApplicationName("Green Island");
        setApplicationVersion("0.1.0");
        setOrganizationName("Maui Project");
        setOrganizationDomain("maui-project.org");
    }

    VShell *loadShell(const QString &name) {
        // Load plugins
        QDir pluginsDir(QStringLiteral("%1/greenisland/shells").arg(INSTALL_PLUGINSDIR));
        foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            VShellPlugin *plugin = qobject_cast<VShellPlugin *>(
                                       loader.instance());
            if (!plugin)
                continue;

            foreach(QString key, plugin->keys()) {
                if (key == name)
                    return plugin->create(key);
            }
        }

        return 0;
    }
};

int main(int argc, char *argv[])
{
    GreenIsland app(argc, argv);

    // Wayland socket from the environment
    QString socketName = QLatin1String(qgetenv("GREENISLAND_SOCKET_NAME"));

    // Shell plugin (defaults to desktop for the moment)
    QString pluginName = QLatin1String("desktop");

    // Command line arguments
    QStringList arguments = QCoreApplication::instance()->arguments();

    // Synthesize touch for unhandled mouse events
    if (arguments.contains(QStringLiteral("--synthesize-touch")))
        app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

    // Shell plugin argument
    int pluginArg = arguments.indexOf(QLatin1String("--plugin"));
    if (pluginArg != -1 && pluginArg + 1 < arguments.size())
        pluginName = arguments.at(pluginArg + 1).toLocal8Bit();

    // Load the shell plugin
    VShell *shell = app.loadShell(pluginName);
    if (!shell)
        qFatal("Unable to run the shell because the '%s' plugin was not found",
               pluginName.toLocal8Bit().constData());

    // Setup compositor and start the shell
    shell->setupCompositor();
    shell->startShell();

    return app.exec();
}

#include "main.moc"
