/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QLoggingCategory>
#include <QtCore/QCommandLineParser>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtQml/QQmlApplicationEngine>

#include "compositor.h"
#include "compositorapp.h"
#include "config.h"
#include "logging.h"
#include "screenmodel.h"
#include "utilities.h"

#if HAVE_SYSTEMD
#  include <systemd/sd-daemon.h>
#endif

int main(int argc, char *argv[])
{
    // Application
    CompositorApp app(argc, argv);
    app.setApplicationName("Green Island");
    app.setApplicationVersion(GREENISLAND_VERSION_STRING);
    app.setOrganizationDomain("maui-project.org");
    app.setOrganizationName("Maui");

    // Check whether XDG_RUNTIME_DIR is ok or not
    GreenIsland::verifyXdgRuntimeDir();

    // Enable debug messages
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("*.warning=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("*.critical=true"));

    // Set message pattern
    qSetMessagePattern("%{message}");

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("Command line parser", "Wayland compositor for the Hawaii desktop environment"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Wayland socket
    QCommandLineOption socketOption(QStringList() << QStringLiteral("s") << QStringLiteral("socket"),
                                    QCoreApplication::translate("Command line parser", "Wayland socket"),
                                    QCoreApplication::translate("Command line parser", "name"));
    parser.addOption(socketOption);

    // Synthesize touch for unhandled mouse events
    QCommandLineOption synthesizeOption(QStringLiteral("synthesize-touch"),
                                        QCoreApplication::translate("Command line parser", "Synthesize touch for unhandled mouse events"));
    parser.addOption(synthesizeOption);

    // Full screen option
    QCommandLineOption fullScreenOption(QStringLiteral("fullscreen"),
                                        QCoreApplication::translate("Command line parser", "Full screen compositor window"));
    parser.addOption(fullScreenOption);

    // Idle time
    QCommandLineOption idleTimeOption(QStringList() << QStringLiteral("i") << QStringLiteral("idle-time"),
                                      QCoreApplication::translate("Command line parser", "Idle time in seconds (at least 5 seconds)"),
                                      QCoreApplication::translate("Command line parser", "secs"));
    idleTimeOption.setDefaultValue("300");
    parser.addOption(idleTimeOption);

    // Screen count
    QCommandLineOption screenCountOption(QStringLiteral("screen-count"),
                                         QCoreApplication::translate("Command line parser", "Screen count"),
                                         QCoreApplication::translate("Command line parser", "num"));
    parser.addOption(screenCountOption);

    // Screen width
    QCommandLineOption widthOption(QStringLiteral("width"),
                                   QCoreApplication::translate("Command line parser", "Screen width"),
                                   QCoreApplication::translate("Command line parser", "num"));
    parser.addOption(widthOption);

    // Screen height
    QCommandLineOption heightOption(QStringLiteral("height"),
                                    QCoreApplication::translate("Command line parser", "Screen height"),
                                    QCoreApplication::translate("Command line parser", "num"));
    parser.addOption(heightOption);

    // Parse command line
    parser.process(app);

    // If a socket is passed it means that we are nesting into
    // another compositor, let's do some checks
    const QString socket = parser.value(socketOption);
    if (!socket.isEmpty()) {
        // We need wayland QPA plugin
        if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
            qWarning() << "By passing the \"--socket\" argument you are requesting to nest"
                       << "this compositor into another, but you forgot to pass "
                       << "also \"-platform wayland\"!";
#if HAVE_SYSTEMD
            sd_notifyf(0, "STATUS=Nesting requested, but no wayland QPA");
#endif
            return 1;
        }
    }

    // Set application attributes
    if (parser.isSet(synthesizeOption))
        app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

    // Fake screen size and count
    if (parser.isSet(screenCountOption)) {
        bool ok;

        int count = parser.value(screenCountOption).toInt(&ok);
        if (!ok || count < 1)
            count = 1;

        int width = parser.value(widthOption).toInt(&ok);
        if (!ok)
            width = 1024;

        int height = parser.value(heightOption).toInt(&ok);
        if (!ok)
            height = 768;

        app.setFakeScreenCount(count);
        app.setFakeScreenSize(QSize(width, height));
    }

    // Create the compositor
    Compositor *compositor = new Compositor(socket);
    compositor->setScreen(QGuiApplication::primaryScreen());

    // Compositor options
    if (parser.isSet(screenCountOption)) {
        compositor->setGeometry(QRect(compositor->screen()->geometry().topLeft(),
                                      app.fakeScreenSize()));
    } else {
        if (parser.isSet(fullScreenOption)) {
            compositor->setGeometry(QGuiApplication::primaryScreen()->availableGeometry());
            compositor->setVisibility(QWindow::FullScreen);
        } else {
            compositor->setGeometry(QRect(compositor->screen()->geometry().topLeft(),
                                          QSize(1920, 1080)));
        }
    }
    int idleInterval = parser.value(idleTimeOption).toInt();
    if (idleInterval >= 5)
        compositor->setIdleInterval(idleInterval * 1000);

    // Show compositor window
    compositor->show();

#if HAVE_SYSTEMD
    sd_notifyf(0,
               "READY=1\n"
               "STATUS=Ready\n"
               "MAINPID=%llu",
               QCoreApplication::applicationPid());
#endif

    return app.exec();
}
