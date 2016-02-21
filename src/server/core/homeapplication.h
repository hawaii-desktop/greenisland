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

#ifndef GREENISLAND_HOMEAPPLICATION_H
#define GREENISLAND_HOMEAPPLICATION_H

#include <QtQml/QQmlApplicationEngine>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class HomeApplicationPrivate;

class GREENISLANDSERVER_EXPORT HomeApplication : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HomeApplication)
    Q_DECLARE_PRIVATE(HomeApplication)
    Q_PROPERTY(QString screenConfiguration READ screenConfiguration WRITE setScreenConfiguration NOTIFY screenConfigurationChanged)
    Q_PROPERTY(bool notificationEnabled READ isNotificationEnabled WRITE setNotificationEnabled NOTIFY notificationEnabledChanged)
public:
    HomeApplication(QObject *parent = Q_NULLPTR);

    QString screenConfiguration() const;
    void setScreenConfiguration(const QString &fileName);

    bool isNotificationEnabled() const;
    void setNotificationEnabled(bool notify);

    QList<QObject *> rootObjects() const;

    void setContextProperty(const QString &name, const QVariant &value);
    void setContextProperty(const QString &name, QObject *object);

    bool load(const QString &shell);
    bool loadUrl(const QUrl &url);

Q_SIGNALS:
    void screenConfigurationChanged(const QString &fileName);
    void notificationEnabledChanged(bool enabled);
    void contextPropertyChanged(const QString &name, const QVariant &value);
    void contextPropertyChanged(const QString &name, QObject *object);
    void objectCreated(QObject *object, const QUrl &url);
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_HOMEAPPLICATION_H

