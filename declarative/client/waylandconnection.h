/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef WAYLANDCONNECTION_H
#define WAYLANDCONNECTION_H

#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlParserStatus>

#include <GreenIsland/Client/ClientConnection>

using namespace GreenIsland::Client;

class WaylandConnection : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString socketName READ socketName WRITE setSocketName NOTIFY socketNameChanged)
    Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false)
    Q_INTERFACES(QQmlParserStatus)
    Q_CLASSINFO("DefaultProperty", "data")
public:
    WaylandConnection(QObject *parent = nullptr);
    ~WaylandConnection();

    QQmlListProperty<QObject> data()
    {
        return QQmlListProperty<QObject>(this, m_objects);
    }

    bool isConnected() const;

    QString socketName() const;
    void setSocketName(const QString &socketName);

    Q_INVOKABLE void start();
    Q_INVOKABLE void flush();

    wl_display *display() const { return m_connection->display(); }

    void classBegin() Q_DECL_OVERRIDE {}
    void componentComplete() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void connectedChanged();
    void socketNameChanged();
    void connected();
    void failed();

private:
    QList<QObject *> m_objects;
    bool m_initialized;
    ClientConnection *m_connection;
    QThread *m_thread;
    QString m_socketName;
};

#endif // WAYLANDCONNECTION_H
