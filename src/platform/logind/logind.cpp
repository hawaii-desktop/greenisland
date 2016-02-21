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
#include <QtCore/private/qobject_p.h>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusUnixFileDescriptor>

#include "logging.h"
#include "logind/logind.h"

#include <qplatformdefs.h>

const static QString login1Service = QLatin1String("org.freedesktop.login1");
const static QString login1Object = QLatin1String("/org/freedesktop/login1");
const static QString login1ManagerInterface = QLatin1String("org.freedesktop.login1.Manager");
const static QString login1SeatInterface = QLatin1String("org.freedesktop.login1.Seat");
const static QString login1SessionInterface = QLatin1String("org.freedesktop.login1.Session");

const static QString dbusService = QLatin1String("org.freedesktop.DBus");
const static QString dbusPropertiesInterface = QLatin1String("org.freedesktop.DBus.Properties");

namespace GreenIsland {

namespace Platform {

class CustomLogind : public Logind
{
public:
    CustomLogind(QObject *parent = 0)
        : Logind(QDBusConnection::systemBus(), parent)
    {
    }
};
Q_GLOBAL_STATIC(CustomLogind, s_logind)

/*
 * LogindPrivate
 */

class LogindPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(Logind)
public:
    LogindPrivate(Logind *self)
        : bus(QDBusConnection::systemBus())
        , watcher(Q_NULLPTR)
        , isConnected(false)
        , hasSessionControl(false)
        , sessionActive(false)
        , vt(-1)
        , q_ptr(self)
    {
    }

    void _q_serviceRegistered()
    {
        Q_Q(Logind);

        // Get the current session as soon as the logind service is registered
        QDBusMessage message =
                QDBusMessage::createMethodCall(login1Service,
                                               login1Object,
                                               login1ManagerInterface,
                                               QLatin1String("GetSessionByPID"));
        message.setArguments(QVariantList() << (quint32)QCoreApplication::applicationPid());

        QDBusPendingReply<QDBusObjectPath> result = bus.asyncCall(message);
        QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(result, q);
        q->connect(callWatcher, &QDBusPendingCallWatcher::finished, q,
                   [q, this](QDBusPendingCallWatcher *w) {
            QDBusPendingReply<QDBusObjectPath> reply = *w;
            w->deleteLater();

            // Skip if we're already connected
            if (isConnected)
                return;

            // Verify the reply
            if (!reply.isValid()) {
                qCDebug(lcLogind, "Session not registered with logind:",
                        qPrintable(reply.error().message()));
                return;
            }

            // Get the session path
            sessionPath = reply.value().path();
            qCDebug(lcLogind) << "Session path:" << sessionPath;

            // We are connected now
            isConnected = true;

            // Listen for lock and unlock signals
            bus.connect(login1Service, sessionPath, login1SessionInterface,
                        QLatin1String("Lock"),
                        q, SIGNAL(lockSessionRequested()));
            bus.connect(login1Service, sessionPath, login1SessionInterface,
                        QLatin1String("Unlock"),
                        q, SIGNAL(unlockSessionRequested()));

            // Listen for properties changed
            bus.connect(login1Service, sessionPath, dbusPropertiesInterface,
                        QLatin1String("PropertiesChanged"),
                        q, SLOT(_q_sessionPropertiesChanged()));

            // Listen for prepare signals
            bus.connect(login1Service, login1Object, login1ManagerInterface,
                        QLatin1String("PrepareForSleep"),
                        q, SIGNAL(prepareForSleep(bool)));
            bus.connect(login1Service, login1Object, login1ManagerInterface,
                        QLatin1String("PrepareForShutdown"),
                        q, SIGNAL(prepareForShutdown(bool)));

            // Get properties
            _q_sessionPropertiesChanged();

            Q_EMIT q->connectedChanged(isConnected);
        });
    }

    void _q_serviceUnregistered()
    {
        Q_Q(Logind);

        // Disconnect prepare signals
        bus.disconnect(login1Service, login1Object, login1ManagerInterface,
                       QLatin1String("PrepareForSleep"),
                       q, SIGNAL(prepareForSleep(bool)));
        bus.disconnect(login1Service, login1Object, login1ManagerInterface,
                       QLatin1String("PrepareForShutdown"),
                       q, SIGNAL(prepareForShutdown(bool)));

        // Connection lost
        isConnected = false;
        Q_EMIT q->connectedChanged(isConnected);

        // Reset properties
        if (sessionActive) {
            sessionActive = false;
            Q_EMIT q->sessionActiveChanged(false);
        }
        if (vt != -1) {
            vt = -1;
            Q_EMIT q->vtNumberChanged(-1);
        }
    }

    void _q_sessionPropertiesChanged()
    {
        if (!isConnected || sessionPath.isEmpty())
            return;

        getSessionActive();
        getVirtualTerminal();
    }

    void _q_devicePaused(uint devMajor, uint devMinor, const QString &type)
    {
        if (QString::compare(type, QLatin1String("pause"), Qt::CaseInsensitive) == 0) {
            QDBusMessage message =
                    QDBusMessage::createMethodCall(login1Service,
                                                   sessionPath,
                                                   login1ManagerInterface,
                                                   QLatin1String("PauseDeviceComplete"));
            message.setArguments(QVariantList() << devMajor << devMinor);

            bus.asyncCall(message);
        }
    }

    void checkServiceRegistration()
    {
        Q_Q(Logind);

        // Get the current session if the logind service is register
        QDBusMessage message =
                QDBusMessage::createMethodCall(dbusService,
                                               QLatin1String("/"),
                                               dbusService,
                                               QLatin1String("ListNames"));

        QDBusPendingReply<QStringList> result = bus.asyncCall(message);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(result, q);
        q->connect(watcher, &QDBusPendingCallWatcher::finished, q,
                   [this](QDBusPendingCallWatcher *w) {
            QDBusPendingReply<QStringList> reply = *w;
            w->deleteLater();

            if (!reply.isValid())
                return;

            if (reply.value().contains(login1Service))
                _q_serviceRegistered();
        });
    }

    QDBusConnection bus;
    QDBusServiceWatcher *watcher;
    bool isConnected;
    bool hasSessionControl;
    QString sessionPath;
    bool sessionActive;
    int vt;
    QVector<int> inhibitFds;

protected:
    Logind *q_ptr;

private:
    void getSessionActive()
    {
        Q_Q(Logind);

        QDBusMessage message =
                QDBusMessage::createMethodCall(login1Service,
                                               sessionPath,
                                               dbusPropertiesInterface,
                                               QLatin1String("Get"));
        message.setArguments(QVariantList() << login1SessionInterface << QStringLiteral("Active"));

        QDBusPendingReply<QVariant> result = bus.asyncCall(message);
        QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(result, q);
        q->connect(callWatcher, &QDBusPendingCallWatcher::finished, q,
                   [q, this](QDBusPendingCallWatcher *w) {
            QDBusPendingReply<QVariant> reply = *w;
            w->deleteLater();

            if (!reply.isValid()) {
                qCWarning(lcLogind, "Failed to get \"Active\" property from session: %s",
                          qPrintable(reply.error().message()));
                return;
            }

            const bool active = reply.value().toBool();
            if (sessionActive != active) {
                sessionActive = active;
                Q_EMIT q->sessionActiveChanged(active);
            }
        });
    }

    void getVirtualTerminal()
    {
        Q_Q(Logind);

        QDBusMessage message =
                QDBusMessage::createMethodCall(login1Service,
                                               sessionPath,
                                               dbusPropertiesInterface,
                                               QLatin1String("Get"));
        message.setArguments(QVariantList() << login1SessionInterface << QStringLiteral("VTNr"));

        QDBusPendingReply<QVariant> result = bus.asyncCall(message);
        QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(result, q);
        q->connect(callWatcher, &QDBusPendingCallWatcher::finished, q,
                   [q, this](QDBusPendingCallWatcher *w) {
            QDBusPendingReply<QVariant> reply = *w;
            w->deleteLater();

            if (!reply.isValid()) {
                qCWarning(lcLogind, "Failed to get \"VTNr\" property from session: %s",
                          qPrintable(reply.error().message()));
                return;
            }

            const uint vtnr = reply.value().toUInt();
            if (vt != static_cast<int>(vtnr)) {
                vt = static_cast<int>(vtnr);
                Q_EMIT q->vtNumberChanged(vt);
            }
        });
    }
};

/*
 * Logind
 */

Logind::Logind(const QDBusConnection &connection, QObject *parent)
    : QObject(*new LogindPrivate(this), parent)
{
    // Initialize and respond to logind service (un)registration
    Q_D(Logind);
    d->bus = connection;
    d->watcher =
            new QDBusServiceWatcher(login1Service, d->bus,
                                    QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                    this);
    connect(d->watcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(_q_serviceRegistered()));
    connect(d->watcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(_q_serviceUnregistered()));

    // Is logind already registered?
    d->checkServiceRegistration();
}

Logind *Logind::instance()
{
    return s_logind();
}

bool Logind::isConnected() const
{
    Q_D(const Logind);
    return d->isConnected;
}

bool Logind::hasSessionControl() const
{
    Q_D(const Logind);
    return d->hasSessionControl;
}

bool Logind::isSessionActive() const
{
    Q_D(const Logind);
    return d->sessionActive;
}

bool Logind::isInhibited() const
{
    Q_D(const Logind);
    return d->inhibitFds.size() > 0;
}

int Logind::vtNumber() const
{
    Q_D(const Logind);
    return d->vt;
}

void Logind::inhibit(const QString &who, const QString &why,
                     InhibitFlags flags, InhibitMode mode)
{
    Q_D(Logind);

    if (!d->isConnected)
        return;

    QStringList what;
    if (flags.testFlag(InhibitShutdown))
        what.append(QStringLiteral("shutdown"));
    if (flags.testFlag(InhibitSleep))
        what.append(QStringLiteral("sleep"));
    if (flags.testFlag(InhibitIdle))
        what.append(QStringLiteral("idle"));
    if (flags.testFlag(InhibitPowerKey))
        what.append(QStringLiteral("handle-power-key"));
    if (flags.testFlag(InhibitSuspendKey))
        what.append(QStringLiteral("handle-suspend-key"));
    if (flags.testFlag(InhibitHibernateKey))
        what.append(QStringLiteral("handle-hibernate-key"));
    if (flags.testFlag(InhibitLidSwitch))
        what.append(QStringLiteral("handle-lid-switch"));

    QString modeStr = mode == Block ? QStringLiteral("block") : QStringLiteral("delay");

    QDBusMessage message =
            QDBusMessage::createMethodCall(login1Service,
                                           login1Object,
                                           login1ManagerInterface,
                                           QLatin1String("Inhibit"));
    message.setArguments(QVariantList() << what.join(':') << who << why << modeStr);

    QDBusPendingReply<QDBusUnixFileDescriptor> result = d->bus.asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(result, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [d, this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QDBusUnixFileDescriptor> reply = *w;
        w->deleteLater();

        if (!reply.isValid()) {
            qCWarning(lcLogind, "Unable to acquire inhibition lock: %s",
                      qPrintable(reply.error().message()));
            return;
        }

        qCDebug(lcLogind) << "Inhibition lock acquired successfully";

        const int fd = ::dup(reply.value().fileDescriptor());
        d->inhibitFds.append(fd);
        if (d->inhibitFds.size() == 1)
            Q_EMIT inhibitedChanged(true);
        Q_EMIT inhibited(fd);
    });
}

void Logind::uninhibit(int fd)
{
    Q_D(Logind);

    if (!d->isConnected || !d->inhibitFds.contains(fd))
        return;

    ::close(fd);
    d->inhibitFds.removeOne(fd);

    if (d->inhibitFds.size() == 0)
        Q_EMIT inhibitedChanged(false);
    Q_EMIT uninhibited(fd);
}

void Logind::lockSession()
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty())
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(login1Service,
                                           d->sessionPath,
                                           login1SessionInterface,
                                           QLatin1String("Lock"));
    d->bus.asyncCall(message);
}

void Logind::unlockSession()
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty())
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(login1Service,
                                           d->sessionPath,
                                           login1SessionInterface,
                                           QLatin1String("Unlock"));
    d->bus.asyncCall(message);
}

void Logind::takeControl()
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty() || d->hasSessionControl)
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(login1Service,
                                           d->sessionPath,
                                           login1SessionInterface,
                                           QLatin1String("TakeControl"));
    message.setArguments(QVariantList() << false);

    QDBusPendingReply<void> result = d->bus.asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(result, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [d, this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<void> reply = *w;
        w->deleteLater();

        if (!reply.isValid()) {
            qCWarning(lcLogind, "Unable to take control of the session: %s",
                      qPrintable(reply.error().message()));
            d->hasSessionControl = false;
            Q_EMIT hasSessionControlChanged(d->hasSessionControl);
            return;
        }

        qCDebug(lcLogind) << "Acquired control of the session";
        d->hasSessionControl = true;
        Q_EMIT hasSessionControlChanged(d->hasSessionControl);

        d->bus.connect(login1Service, d->sessionPath, login1SessionInterface,
                       QLatin1String("PauseDevice"),
                       this, SLOT(_q_devicePaused(uint,uint,QString)));
    });
}

void Logind::releaseControl()
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty() || !d->hasSessionControl)
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(login1Service,
                                           d->sessionPath,
                                           login1SessionInterface,
                                           QLatin1String("ReleaseControl"));
    d->bus.asyncCall(message);

    qCDebug(lcLogind) << "Released control of the session";
    d->hasSessionControl = false;
    Q_EMIT hasSessionControlChanged(d->hasSessionControl);
}

int Logind::takeDevice(const QString &fileName)
{
    Q_D(Logind);

    struct stat st;
    if (::stat(qPrintable(fileName), &st) < 0) {
        qCWarning(lcLogind, "Failed to stat: %s", qPrintable(fileName));
        return -1;
    }

    QDBusMessage message =
            QDBusMessage::createMethodCall(login1Service,
                                           d->sessionPath,
                                           login1SessionInterface,
                                           QLatin1String("TakeDevice"));
    message.setArguments(QVariantList()
                         << QVariant(major(st.st_rdev))
                         << QVariant(minor(st.st_rdev)));

    // Block until the device is taken
    QDBusMessage reply = d->bus.call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(lcLogind, "Failed to take device \"%s\": %s",
                  qPrintable(fileName), qPrintable(reply.errorMessage()));
        return -1;
    }

    const int fd = reply.arguments().first().value<QDBusUnixFileDescriptor>().fileDescriptor();
    return ::dup(fd);
}

void Logind::releaseDevice(int fd)
{
    Q_D(Logind);

    struct stat st;
    if (::fstat(fd, &st) < 0) {
        qCWarning(lcLogind, "Failed to stat the file descriptor");
        return;
    }

    QDBusMessage message =
            QDBusMessage::createMethodCall(login1Service,
                                           d->sessionPath,
                                           login1SessionInterface,
                                           QLatin1String("ReleaseDevice"));
    message.setArguments(QVariantList()
                         << QVariant(major(st.st_rdev))
                         << QVariant(minor(st.st_rdev)));

    d->bus.asyncCall(message);
}

void Logind::switchTo(quint32 vt)
{
    Q_D(Logind);

    QDBusMessage message =
            QDBusMessage::createMethodCall(login1Service,
                                           QLatin1String("/org/freedesktop/login1/seat/self"),
                                           login1SeatInterface,
                                           QLatin1String("SwitchTo"));
    message.setArguments(QVariantList() << QVariant(vt));

    d->bus.asyncCall(message);
}

} // namespace Platform

} // namespace GreenIsland

#include "moc_logind.cpp"
