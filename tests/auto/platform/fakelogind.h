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

#ifndef FAKELOGIND_H
#define FAKELOGIND_H

#include <QtCore/QObject>
#include <QtDBus/QDBusObjectPath>

class FakeLogindSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool Active READ isActive)
    Q_PROPERTY(uint VTNr READ vtNumber)
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Session")
public:
    explicit FakeLogindSession(const QString &path, QObject *parent = 0);
    virtual ~FakeLogindSession();

    const QString &path();

    bool isActive() const;
    quint32 vtNumber() const;

public Q_SLOTS:
    Q_SCRIPTABLE void TakeControl(bool force);
    Q_SCRIPTABLE void ReleaseControl();

Q_SIGNALS:
    Q_SCRIPTABLE void Lock();
    Q_SCRIPTABLE void Unlock();

private:
    QString m_path;
};

class FakeLogind : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Manager")
public:
    explicit FakeLogind(QObject *parent = 0);
    virtual ~FakeLogind();

    // Methods to trigger signals
    void doLock();
    void doUnlock();
    void doPrepareForSleep(bool before);
    void doPrepareForShutdown(bool before);

public Q_SLOTS:
    Q_SCRIPTABLE QDBusObjectPath GetSessionByPID(quint32 pid);
    Q_SCRIPTABLE int TakeDevice(int maj, int min);
    Q_SCRIPTABLE void ReleaseDevice(int maj, int min);

Q_SIGNALS:
    Q_SCRIPTABLE void PrepareForSleep(bool before);
    Q_SCRIPTABLE void PrepareForShutdown(bool before);

private:
    FakeLogindSession *m_session;
};

#endif // FAKELOGIND_H
