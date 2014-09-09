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
#include <QDebug>

#include "cmakedirs.h"
#include "processcontroller.h"

#define FULLSCREEN_SHELL_SOCKET "greenisland-fsh-"
#define GREENISLAND_SOCKET "greenisland-"

ProcessController::ProcessController(QObject *parent)
    : QObject(parent)
    , m_fullScreenShell(Q_NULLPTR)
{
    // Compositor process
    m_compositor = new QProcess(this);
    m_compositor->setProgram(QStringLiteral(INSTALL_BINDIR "/greenisland"));
    connect(m_compositor, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(compositorFinished(int,QProcess::ExitStatus)));

    // Calculate Wayland socket suffix
    m_socketSuffix = randomString();
}

bool ProcessController::isFullScreenShellEnabled() const
{
    return m_fullScreenShell != Q_NULLPTR;
}

void ProcessController::setFullScreenShellEnabled(bool value)
{
    if (value && m_fullScreenShell)
        return;

    if (!value && m_fullScreenShell) {
        compositorFinished(0, QProcess::NormalExit);
        return;
    }

    if (value) {
        m_fullScreenShell = new QProcess(this);
        m_fullScreenShell->setProgram(QStringLiteral("weston"));
        m_fullScreenShell->setArguments(QStringList()
                                        << QStringLiteral("--shell=fullscreen-shell.so")
                                        << QStringLiteral("--socket=" FULLSCREEN_SHELL_SOCKET) + m_socketSuffix);
        connect(m_fullScreenShell, SIGNAL(started()), this, SLOT(startCompositor()));
    }
}

void ProcessController::start()
{
    // Run the full screen shell compositor if enabled
    if (m_fullScreenShell) {
        m_fullScreenShell->start();

        if (!m_fullScreenShell->waitForStarted())
            qFatal("Full Screen Shell compositor cannot be started, aborting...");

        return;
    }

    // Run the compositor
    startCompositor();
}

QString ProcessController::randomString() const
{
    // Courtesy of Merlin069 from:
    // http://stackoverflow.com/questions/18862963/qt-c-random-string-generation

    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 12;

    QString randomString;
    for(int i = 0; i<randomStringLength; ++i) {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }

    return randomString;
}

void ProcessController::detect()
{
    // No need to detect anything if full screen shell is forced
    if (isFullScreenShellEnabled())
        return;

    // X11
    if (qEnvironmentVariableIsSet("DISPLAY")) {
        m_compositor->setArguments(QStringList()
                                   << QStringLiteral("-platform")
                                   << QStringLiteral("xcb")
                                   << QStringLiteral("--fake-screen")
                                   << QStringLiteral("../data/kscreen/one-1024x768.json"));
        return;
    }

    // Assume it is running on KMS, therefore enable full screen shell
    setFullScreenShellEnabled(true);
}

void ProcessController::startCompositor()
{
    // Detect the environment
    detect();

    // Pass arguments for full screen shell
    if (isFullScreenShellEnabled()) {
        m_compositor->setArguments(QStringList()
                                   << QStringLiteral("-platform")
                                   << QStringLiteral("wayland")
                                   << QStringLiteral("--socket=" GREENISLAND_SOCKET) + m_socketSuffix);

        QProcessEnvironment env;
        env.insert(QStringLiteral("WAYLAND_DISPLAY"),
                   QStringLiteral(FULLSCREEN_SHELL_SOCKET) + m_socketSuffix);
        m_compositor->setProcessEnvironment(env);
    }

    // Start the process
    m_compositor->start();
    if (!m_compositor->waitForStarted()) {
        // Compositor failed to start, kill full screen shell and terminate
        qFatal("Compositor won't start, aborting...");
        compositorFinished(0, QProcess::NormalExit);
    }
}

void ProcessController::compositorFinished(int code, const QProcess::ExitStatus &status)
{
    Q_UNUSED(status);

    if (code != 0)
        qWarning() << "Compositor finished with exit code" << code;

    // Whathever the reason why it finished is we need to quit the
    // full screen shell compositor, if any
    if (m_fullScreenShell) {
        m_fullScreenShell->terminate();
        if (!m_fullScreenShell->waitForFinished())
            m_fullScreenShell->kill();
        m_fullScreenShell->deleteLater();
        m_fullScreenShell = Q_NULLPTR;
    }

    // Quit
    qApp->quit();
}

void ProcessController::fullScreenShellFinished(int code, const QProcess::ExitStatus &status)
{
    Q_UNUSED(status);

    if (code != 0)
        qWarning() << "Full screen shell finished with exit code" << code;

    // Kill the compositor if for some reason is still running
    m_compositor->terminate();
    if (!m_compositor->waitForFinished())
        m_compositor->kill();

    // Quit
    qApp->quit();
}
