/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
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
#include <VibeCore/VCommandOptions>

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

        // Synthesize touch events from mouse input
        if (arguments().contains(QStringLiteral("-synthesizetouch")))
            setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);
    }

    VShell *loadShell(const QString &name, const QString &socketName) {
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
                    return plugin->create(key, socketName);
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

    // Parse command line
    VCommandOptions options;
    options.add("socketname", "set the socket name", VCommandOptions::Required);
    options.add("plugin", "set the shell plugin to use", VCommandOptions::Required);
    options.add("synthesizetouch", "synthesize touch for unhandled mouse events");
    options.add("help", "show this help text");
    options.alias("help", "h");
    options.parse(QCoreApplication::arguments());
    if (options.count("help") || options.showUnrecognizedWarning()) {
        options.showUsage();
        return 1;
    }

    // Synthesize touch for unhandled mouse events
    if (options.count("synthesizetouch"))
        app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

    // Socket argument
    if (options.count("socketname"))
        socketName = options.value("socketname").toString();

    // Shell plugin argument
    if (options.count("plugin"))
        pluginName = options.value("plugin").toString();

    // Load the shell plugin
    VShell *shell = app.loadShell(pluginName, socketName);
    if (!shell)
        qFatal("Unable to run the shell, aborting...");

    // Setup compositor and start the shell
    shell->setupCompositor();
    shell->startShell();

    return app.exec();
}

#include "main.moc"
