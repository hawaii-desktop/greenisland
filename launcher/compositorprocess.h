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

#ifndef COMPOSITORPROCESS_H
#define COMPOSITORPROCESS_H

#include <QtCore/QObject>
#include <QtCore/QProcessEnvironment>

class QFileSystemWatcher;

class CompositorProcess : public QObject
{
    Q_OBJECT
public:
    CompositorProcess(QObject *parent = 0);
    ~CompositorProcess();

    QString socketName() const;
    void setSocketName(const QString &name);

    void setProgram(const QString &prog);
    void setArguments(const QStringList &args);

    QProcessEnvironment environment() const;
    void setEnvironment(const QProcessEnvironment &env);

Q_SIGNALS:
    void started();
    void stopped();
    void finished();

public Q_SLOTS:
    void start();
    void stop();

private:
    QProcess *m_process;
    QString m_prog;
    QStringList m_args;
    QProcessEnvironment m_env;

    int m_retries;
    QString m_xdgRuntimeDir;
    QString m_socketName;
    QFileSystemWatcher *m_watcher;

private Q_SLOTS:
    void processError(QProcess::ProcessError error);
    void processFinished(int exitCode, QProcess::ExitStatus status);
    void socketAvailable();
    bool respawn();
};

#endif // COMPOSITORPROCESS_H
