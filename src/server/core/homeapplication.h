/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL2.1+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

