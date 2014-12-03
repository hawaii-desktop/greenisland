/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QLoggingCategory>

#include "config.h"
#include "processcontroller.h"

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

int main(int argc, char *argv[])
{
    // Application
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Green Island"));
    app.setApplicationVersion(QStringLiteral(GREENISLAND_VERSION_STRING));
    app.setOrganizationDomain(QStringLiteral("maui-project.org"));
    app.setOrganizationName(QStringLiteral("Maui"));

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(TR("Prepares the environment for Green Island and launches it"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Full screen shell
    QCommandLineOption fullScreenShellOption(QStringLiteral("fullscreen-shell"),
                                             TR("Nest Green Island into a full screen shell compositor"));
    parser.addOption(fullScreenShellOption);

    // Compositor package
    QCommandLineOption pluginOption(QStringList() << QStringLiteral("p") << QStringLiteral("compositor-plugin"),
                                    TR("Force loading the given compositor plugin"),
                                    QStringLiteral("plugin"));
    parser.addOption(pluginOption);

    // Parse command line
    parser.process(app);

    // Process controller that manages the compositor
    ProcessController processController;

    // Force full screen shell if requested
    processController.setFullScreenShellEnabled(parser.isSet(fullScreenShellOption));

    // Pass additional arguments
    processController.setPlugin(parser.value(pluginOption));

    // Start the compositor
    processController.start();

    return app.exec();
}

