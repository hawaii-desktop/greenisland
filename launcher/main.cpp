/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL3-HAWAII$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 or any later version accepted
 * by Pier Luigi Fiorini, which shall act as a proxy defined in Section 14
 * of version 3 of the license.
 *
 * Any modifications to this file must keep this entire header intact.
 *
 * The interactive user interfaces in modified source and object code
 * versions of this program must display Appropriate Legal Notices,
 * as required under Section 5 of the GNU General Public License version 3.
 *
 * In accordance with Section 7(b) of the GNU General Public License
 * version 3, these Appropriate Legal Notices must retain the display of the
 * "Powered by Hawaii" logo.  If the display of the logo is not reasonably
 * feasible for technical reasons, the Appropriate Legal Notices must display
 * the words "Powered by Hawaii".
 *
 * In accordance with Section 7(c) of the GNU General Public License
 * version 3, modified source and object code versions of this program
 * must be marked in reasonable ways as different from the original version.
 *
 * In accordance with Section 7(d) of the GNU General Public License
 * version 3, neither the "Hawaii" name, nor the name of any project that is
 * related to it, nor the names of its contributors may be used to endorse or
 * promote products derived from this software without specific prior written
 * permission.
 *
 * In accordance with Section 7(e) of the GNU General Public License
 * version 3, this license does not grant any license or rights to use the
 * "Hawaii" name or logo, nor any other trademark.
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
#include "cmakedirs.h"
#include "compositorlauncher.h"

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

int main(int argc, char *argv[])
{
    // Application
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Green Island"));
    app.setApplicationVersion(QStringLiteral(GREENISLAND_VERSION_STRING));
    app.setOrganizationName(QStringLiteral("Hawaii"));
    app.setOrganizationDomain(QStringLiteral("hawaiios.org"));

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(TR("Prepares the environment for Green Island and launches it"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Mode
    QCommandLineOption modeOption(QStringList() << QStringLiteral("m") << QStringLiteral("mode"),
                                  TR("Specify mode (eglfs, hwcomposer, nested, x11, wayland)."),
                                  TR("mode"));
    parser.addOption(modeOption);

    // Applications to start
    QCommandLineOption exeOption(QStringLiteral("execute"),
                                 TR("Command to execute."), TR("command"));
    parser.addOption(exeOption);

    // Parse command line
    parser.process(app);

    // Assume greenisland by default
    QString program = QStringLiteral(INSTALL_BINDIR "/greenisland");
    QStringList arguments;
    if (parser.isSet(exeOption)) {
        arguments = parser.value(exeOption).split(QLatin1Char(' '),
                                                  QString::SkipEmptyParts);
        program = arguments.takeFirst();
    }

    // Start the compositor
    CompositorLauncher launcher(program, arguments);
    if (parser.isSet(modeOption)) {
        QString modeString = parser.value(modeOption);
        CompositorLauncher::Mode mode = CompositorLauncher::UnknownMode;
        if (modeString == QStringLiteral("eglfs"))
            mode = CompositorLauncher::EglFSMode;
        else if (modeString == QStringLiteral("hwcomposer"))
            mode = CompositorLauncher::HwComposerMode;
        else if (modeString == QStringLiteral("nested"))
            mode = CompositorLauncher::NestedMode;
        else if (modeString == QStringLiteral("x11"))
            mode = CompositorLauncher::X11Mode;
        else if (modeString == QStringLiteral("wayland"))
            mode = CompositorLauncher::WaylandMode;
        else
            qFatal("Invalid mode \"%s\" specified, accepted values are: "
                   "eglfs, hwcomposer, nested, x11, wayland",
                   qPrintable(modeString));
        launcher.setMode(mode);
    }
    launcher.start();

    return app.exec();
}

