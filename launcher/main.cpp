/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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

