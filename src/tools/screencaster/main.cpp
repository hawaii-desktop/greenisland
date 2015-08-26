/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2014 Jolla Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Original Author(s):
 *    Giulio Camuffo <giulio.camuffo@jollamobile.com>
 *
 * $BEGIN_LICENSE:LGPL2.1$
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging
 * of this file.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtGui/QGuiApplication>

#include "config.h"
#include "screencaster.h"

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("greenisland-screencaster"));
    app.setApplicationVersion(QStringLiteral(GREENISLAND_VERSION_STRING));
    app.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Green Island Screencast recorder"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Output file name
    QCommandLineOption outputOption(QStringList() << QStringLiteral("o") << QStringLiteral("output"),
                               TR("Output file name"), TR("filename"));
    parser.addOption(outputOption);

    // Number of frames to capture
    QCommandLineOption framesOption(QStringList() << QStringLiteral("n") << QStringLiteral("number-of-frames"),
                                    TR("Number of frames to record"), TR("number-of-frames"));
    parser.addOption(framesOption);

    // Parse command line
    parser.process(app);

    // Need to be running with wayland QPA
    if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
        qCritical() << "greenisland-screencast requires a Wayland session";
        return 1;
    }

    // Arguments check
    if (!parser.isSet(outputOption)) {
        qCritical() << "Output file name is mandatory!";
        return 1;
    }

    // Output file name
    const QString fileName = parser.value(outputOption);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to open output file for writing";
        return 1;
    }

    unsigned int numberOfFrames = 0;
    if (parser.isSet(framesOption))
        numberOfFrames = parser.value(framesOption).toInt();

    // Start the screen caster
    ScreenCaster screenCaster(&file, numberOfFrames);
    return app.exec();
}
