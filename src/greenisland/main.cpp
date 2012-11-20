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

#include <QDebug>
#include <QScreen>
#include <QStringList>
#include <QWindow>

#include <VCompositor>

#include "greenisland.h"

int main(int argc, char *argv[])
{
    // Assume the xcb platform if the DISPLAY environment variable is defined,
    // otherwise go for kms
    if (!qgetenv("DISPLAY").isEmpty())
        setenv("QT_QPA_PLATFORM", "xcb", 0);
    else {
        setenv("QT_QPA_PLATFORM", "kms", 0);
        setenv("QT_QPA_GENERIC_PLUGINS", "evdevmouse,evdevkeyboard,evdevtouch", 0);
    }

    GreenIsland app(argc, argv);

    // Shell plugin (defaults to desktop for the moment)
    QString pluginName = QLatin1String("desktop");

    // Command line arguments
    QStringList arguments = QCoreApplication::instance()->arguments();

    // Usage instructions
    if (arguments.contains(QStringLiteral("-h")) || arguments.contains(QStringLiteral("--help"))) {
        qDebug() << "Usage: greenisland [options]";
        qDebug() << "Arguments are:";
        qDebug() << "\t--fullscreen\t\trun in fullscreen mode";
        qDebug() << "\t--synthesize-touch\tsynthesize touch for unhandled mouse events";
        qDebug() << "\t--plugin NAME\t\tuse the NAME shell plugin (default 'desktop')";
        return 0;
    }

    // Synthesize touch for unhandled mouse events
    if (arguments.contains(QStringLiteral("--synthesize-touch")))
        app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

    // Shell plugin argument
    int pluginArg = arguments.indexOf(QLatin1String("--plugin"));
    if (pluginArg != -1 && pluginArg + 1 < arguments.size())
        pluginName = arguments.at(pluginArg + 1).toLocal8Bit();

    // Load the compositor plugin
    VCompositor *compositor = app.loadCompositor(pluginName);
    if (!compositor)
        qFatal("Unable to run the compositor because the '%s' plugin was not found",
               pluginName.toLocal8Bit().constData());

    // Ensure the compositor renders into a window
    if (!compositor->window())
        qFatal("The compositor '%s' doesn't render into a window",
               pluginName.toLocal8Bit().constData());

    // Set window title
    compositor->window()->setWindowTitle(QLatin1String("Green Island"));

    // Run the shell
    compositor->runShell();

    // Show the compositor
    if (arguments.contains(QStringLiteral("--fullscreen"))) {
        compositor->window()->setGeometry(QGuiApplication::primaryScreen()->geometry());
        compositor->window()->showFullScreen();
    } else {
        compositor->window()->setGeometry(QGuiApplication::primaryScreen()->availableGeometry());
        compositor->window()->showMaximized();
    }

    return app.exec();
}
