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
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileSystemWatcher>

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
    m_compositor->setProcessChannelMode(QProcess::ForwardedChannels);
    m_compositor->setProgram(QStringLiteral(INSTALL_BINDIR "/greenisland"));
    connect(m_compositor, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(compositorFinished(int,QProcess::ExitStatus)));

    // Wayland sockets
    QString random = randomString();
    m_compositorSocket = QStringLiteral(GREENISLAND_SOCKET) + random;
    m_fullScreenShellSocket = QStringLiteral(FULLSCREEN_SHELL_SOCKET) + random;
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
        m_fullScreenShell->setProcessChannelMode(QProcess::ForwardedChannels);
        m_fullScreenShell->setProgram(QStringLiteral("weston"));
        m_fullScreenShell->setArguments(QStringList()
                                        << QStringLiteral("--shell=fullscreen-shell.so")
                                        << QStringLiteral("--socket=") + m_fullScreenShellSocket);

        m_fullScreenShellWatcher = new QFileSystemWatcher(this);
        m_fullScreenShellWatcher->addPath(QString::fromUtf8(qgetenv("XDG_RUNTIME_DIR")));
        connect(m_fullScreenShellWatcher, SIGNAL(directoryChanged(QString)),
                this, SLOT(directoryChanged(QString)));
    }
}

QString ProcessController::shell() const
{
    return m_shell;
}

void ProcessController::setShell(const QString &shell)
{
    m_shell = shell;
}

void ProcessController::start()
{
    // Run the full screen shell compositor if enabled
    if (m_fullScreenShell) {
        qDebug() << "Running:" << qPrintable(m_fullScreenShell->program())
                 << qPrintable(m_fullScreenShell->arguments().join(" "));
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

void ProcessController::startCompositor()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Pass arguments for full screen shell or another mode
    if (isFullScreenShellEnabled()) {
        m_compositor->setArguments(QStringList()
                                   << QStringLiteral("--socket=") + m_compositorSocket);

        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("wayland"));
        env.insert(QStringLiteral("WAYLAND_DISPLAY"), m_fullScreenShellSocket);
        if (qEnvironmentVariableIsSet("DISPLAY") && !isFullScreenShellEnabled())
            env.insert(QStringLiteral("QT_XCB_GL_INTEGRATION"), QStringLiteral("xcb_egl"));
        m_compositor->setProcessEnvironment(env);
    } else if (qEnvironmentVariableIsSet("DISPLAY")) {
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("xcb"));
    }

    if (!m_shell.isEmpty())
        m_compositor->setArguments(m_compositor->arguments()
                                   << QStringLiteral("--shell") << m_shell);

    // Start the process
    qDebug() << "Running:" << qPrintable(m_compositor->program())
             << qPrintable(m_compositor->arguments().join(" "));
    m_compositor->start();
    if (!m_compositor->waitForStarted()) {
        // Compositor failed to start, kill full screen shell and terminate
        qFatal("Compositor won't start, aborting...");
        compositorFinished(0, QProcess::NormalExit);
    }
}

void ProcessController::directoryChanged(const QString &path)
{
    Q_UNUSED(path);

    // Don't start until the full screen shell compositor is up
    const QString socketFileName = path + QStringLiteral("/") +
            m_fullScreenShellSocket;
    if (!QFile::exists(socketFileName))
        return;

    // Socket is here, let's bring the compositor up but first disconnect
    // and destroy the file system watcher otherwise this slot gets
    // called over and over again
    m_fullScreenShellWatcher->disconnect(this);
    m_fullScreenShellWatcher->deleteLater();
    startCompositor();
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
