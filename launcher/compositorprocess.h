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
