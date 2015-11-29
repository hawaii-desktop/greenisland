/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#ifndef GREENISLAND_APPLICATIONMANAGER_H
#define GREENISLAND_APPLICATIONMANAGER_H

#include <QtCore/QObject>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class ApplicationManagerPrivate;

class GREENISLANDSERVER_EXPORT ApplicationManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ApplicationManager)
public:
    ApplicationManager(QObject *parent = Q_NULLPTR);

    /*!
     * \brief Registers the \a window.
     */
    Q_INVOKABLE void registerWindow(QObject *window);

    /*!
     * \brief Unregisters the \a window.
     */
    Q_INVOKABLE void unregisterWindow(QObject *window);

    /*!
     * \brief Returns whether the application is registered or not.
     * \param appId Application identifier.
     */
    Q_INVOKABLE bool isRegistered(const QString &appId) const;

    /*!
     * \brief Quits an application.
     * \param appId Application identifier.
     */
    Q_INVOKABLE void quit(const QString &appId);

Q_SIGNALS:
    /*!
     * \brief An application has been added.
     * \param appId Application identifier.
     * \param pid Process identifier.
     */
    void applicationAdded(const QString &appId, pid_t pid);

    /*!
     * \brief An application has been removed.
     * \param appId Application identifier.
     * \param pid Process identifier.
     */
    void applicationRemoved(const QString &appId, pid_t);

    /*!
     * \brief An application got focused.
     * \param appId Application identifier.
     */
    void applicationFocused(const QString &appId);

    /*!
     * \brief An application got unfocused.
     * \param appId Application identifier.
     */
    void applicationUnfocused(const QString &appId);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_appIdChanged(const QString &appId))
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_APPLICATIONMANAGER_H
