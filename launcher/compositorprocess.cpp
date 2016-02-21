/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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
#include <QtCore/QFile>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QTimer>

#include "compositorlauncher.h"
#include "compositorprocess.h"

#include <signal.h>

CompositorProcess::CompositorProcess(QObject *parent)
    : QObject(parent)
    , m_process(Q_NULLPTR)
    , m_retries(5)
    , m_xdgRuntimeDir(QString::fromUtf8(qgetenv("XDG_RUNTIME_DIR")))
    , m_watcher()
{
}

CompositorProcess::~CompositorProcess()
{
    stop();
}

QString CompositorProcess::socketName() const
{
    return m_socketName;
}

void CompositorProcess::setSocketName(const QString &name)
{
    m_socketName = name;
}

void CompositorProcess::setProgram(const QString &prog)
{
    m_prog = prog;
}

void CompositorProcess::setArguments(const QStringList &args)
{
    m_args = args;
}

QProcessEnvironment CompositorProcess::environment() const
{
    return m_env;
}

void CompositorProcess::setEnvironment(const QProcessEnvironment &env)
{
    m_env = env;
}

void CompositorProcess::start()
{
    qCDebug(COMPOSITOR)
            << "Starting:"
            << qPrintable(m_prog)
            << qPrintable(m_args.join(' '));
    if (m_process) {
        qCWarning(COMPOSITOR) << "Process already running";
        return;
    }

    m_process = new QProcess;
    m_process->setProgram(m_prog);
    m_process->setArguments(m_args);
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);
    m_process->setProcessEnvironment(m_env);
    connect(m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(processError(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(processFinished(int,QProcess::ExitStatus)));

    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(socketAvailable()));
    m_watcher->addPath(m_xdgRuntimeDir);

    m_process->start();
}

void CompositorProcess::stop()
{
    if (!m_process)
        return;

    qCDebug(COMPOSITOR)
            << "Stopping:"
            << qPrintable(m_process->program())
            << qPrintable(m_process->arguments().join(' '));

    disconnect(m_watcher, SIGNAL(directoryChanged(QString)),
               this, SLOT(socketAvailable()));
    m_watcher->deleteLater();
    m_watcher = Q_NULLPTR;

    m_process->terminate();
    if (!m_process->waitForFinished())
        m_process->kill();
    m_process->deleteLater();
    m_process = Q_NULLPTR;
    Q_EMIT stopped();
}

void CompositorProcess::processError(QProcess::ProcessError error)
{
    Q_UNUSED(error)

    qCWarning(COMPOSITOR)
            << "Process" << m_process->program()
            << "had an error:" << m_process->errorString();

    // Respawn process
    if (respawn())
        return;

    // Compositor failed to start, kill full screen shell and terminate
    qCritical(COMPOSITOR)
            << "Process" << m_prog
            << "won't start, aborting...";
    QCoreApplication::quit();
}

void CompositorProcess::processFinished(int exitCode, QProcess::ExitStatus status)
{
    if (exitCode == 0) {
        qCWarning(COMPOSITOR)
                << "Process" << m_prog
                << "finished successfully";
        Q_EMIT finished();
        m_process->deleteLater();
        m_process = Q_NULLPTR;
    } else if (status == QProcess::NormalExit) {
        qCWarning(COMPOSITOR)
                << "Process" << m_prog
                << "finished with exit code" << exitCode;
        Q_EMIT stopped();
        m_process->deleteLater();
        m_process = Q_NULLPTR;
    }
}

void CompositorProcess::socketAvailable()
{
    if (m_socketName.isEmpty())
        return;

    // Check whether the socket was created
    const QString fileName = m_xdgRuntimeDir + QStringLiteral("/") + m_socketName;
    qCDebug(COMPOSITOR) << "Checking for Wayland socket:" << fileName;
    if (!QFile::exists(fileName))
        return;

    // Do not listen anymore, we found the socket
    m_watcher->removePath(m_xdgRuntimeDir);
    disconnect(m_watcher, SIGNAL(directoryChanged(QString)),
               this, SLOT(socketAvailable()));
    qCDebug(COMPOSITOR) << "Socket" << fileName << "found";
    Q_EMIT started();
}

bool CompositorProcess::respawn()
{
    if (m_retries < 1)
        return false;

    // QProcess will emit finished() after error() but this will make the
    // session quit in nested mode because of the way the full screen shell
    // compositor is connected to the real compositor
    disconnect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
               this, SLOT(processFinished(int,QProcess::ExitStatus)));

    qCDebug(COMPOSITOR)
            << "Process" << m_prog
            << "retries left:" << m_retries;
    m_retries--;

    // Remove watcher
    disconnect(m_watcher, SIGNAL(directoryChanged(QString)),
               this, SLOT(socketAvailable()));
    m_watcher->deleteLater();
    m_watcher = Q_NULLPTR;

    // Destroy the old process
    m_process->deleteLater();
    m_process = Q_NULLPTR;

    // Restart the process after 3 seconds
    QTimer::singleShot(3000, this, SLOT(start()));

    return true;
}

#include "moc_compositorprocess.cpp"
