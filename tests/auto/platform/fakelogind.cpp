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

#include <QtDBus/QDBusConnection>

#include "fakelogind.h"

/*
 * FakeLogindSession
 */

FakeLogindSession::FakeLogindSession(const QString &path, QObject *parent)
    : QObject(parent)
    , m_path(path)
{
    QDBusConnection::sessionBus().registerObject(
                m_path, this, QDBusConnection::ExportScriptableContents);
}

FakeLogindSession::~FakeLogindSession()
{
    QDBusConnection::sessionBus().unregisterObject(m_path);
}

const QString &FakeLogindSession::path()
{
    return m_path;
}

bool FakeLogindSession::isActive() const
{
    return true;
}

quint32 FakeLogindSession::vtNumber() const
{
    return 1;
}

void FakeLogindSession::TakeControl(bool force)
{
    Q_UNUSED(force);
}

void FakeLogindSession::ReleaseControl()
{
}

/*
 * FakeLogind
 */

FakeLogind::FakeLogind(QObject *parent)
    : QObject(parent)
    , m_session(new FakeLogindSession(QLatin1String("/org/freedesktop/login1/session/_1"), this))
{
    QDBusConnection::sessionBus().registerObject(
                QLatin1String("/org/freedesktop/login1"), this,
                QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerService(
                QLatin1String("org.freedesktop.login1"));
}

FakeLogind::~FakeLogind()
{
    QDBusConnection::sessionBus().unregisterObject(
                QLatin1String("/org/freedesktop/login1"));
    QDBusConnection::sessionBus().unregisterService(
                QLatin1String("org.freedesktop.login1"));
}

void FakeLogind::doLock()
{
    Q_EMIT m_session->Lock();
}

void FakeLogind::doUnlock()
{
    Q_EMIT m_session->Unlock();
}

void FakeLogind::doPrepareForSleep(bool before)
{
    Q_EMIT PrepareForSleep(before);
}

void FakeLogind::doPrepareForShutdown(bool before)
{
    Q_EMIT PrepareForShutdown(before);
}

QDBusObjectPath FakeLogind::GetSessionByPID(quint32 pid)
{
    Q_UNUSED(pid);
    return QDBusObjectPath(m_session->path());
}

int FakeLogind::TakeDevice(int maj, int min)
{
return -1;
}

void FakeLogind::ReleaseDevice(int maj, int min)
{

}

#include "moc_fakelogind.cpp"
