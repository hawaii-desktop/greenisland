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
