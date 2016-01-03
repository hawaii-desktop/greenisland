/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtGui/QGuiApplication>

#include "config.h"
#include "application.h"

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

int main(int argc, char *argv[])
{
    // Setup the application
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("greenisland-screencaster"));
    app.setApplicationVersion(QStringLiteral(GREENISLAND_VERSION_STRING));
    app.setQuitOnLastWindowClosed(false);

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Command line screencaster"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Output file name
    QCommandLineOption outputOption(QStringList() << QStringLiteral("f") << QStringLiteral("filename"),
                                    TR("Output file name."), TR("filename"));
    parser.addOption(outputOption);

    // Still images only
    QCommandLineOption stillOption(QStringList() << QStringLiteral("s") << QStringLiteral("still-images"),
                                   TR("Records frames into PNG images."));
    parser.addOption(stillOption);

    // Maximum number of frames to capture
    QCommandLineOption framesOption(QStringList() << QStringLiteral("n") << QStringLiteral("nframes"),
                                    TR("Maximum number of frames to record."), TR("number-of-frames"));
    parser.addOption(framesOption);

    // Parse command line
    parser.process(app);

    // Need to be running with wayland QPA
    if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
        qCritical("greenisland-screencast requires a Wayland session");
        return 1;
    }

    // Arguments check
    if (!parser.isSet(outputOption) && !parser.isSet(stillOption)) {
        qCritical("You must specify either an output file name or to record still images");
        return 1;
    }
    if (parser.isSet(outputOption) && parser.isSet(stillOption)) {
        qCritical("The --filename and --still-images arguments conflict with eath other");
        return 1;
    }

    // Maximum number of frames to capture (if specified must be at least 30)
    int frames = 0;
    if (parser.isSet(framesOption)) {
        frames = parser.value(framesOption).toInt();
        if (frames < 30)
            frames = 0;
    }

    // Start the screen caster
    Application *screencaster = new Application(parser.value(outputOption), frames,
                                                parser.isSet(stillOption), &app);
    QGuiApplication::postEvent(screencaster, new StartupEvent());

    return app.exec();
}
