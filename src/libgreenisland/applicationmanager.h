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

#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QtCore/QObject>

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class ApplicationInfo;
class ApplicationManagerPrivate;
class ClientWindow;

class GREENISLAND_EXPORT ApplicationManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ApplicationManager)
public:
    ApplicationManager(QObject *parent);
    ~ApplicationManager();

    /*!
     * \brief Returns whether the application is registered or not.
     * \param appId Application identifier.
     */
    Q_INVOKABLE bool isRegistered(const QString &appId) const;

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

    /*!
     * \brief An application window was mapped.
     * \param window Application window.
     */
    void windowMapped(ClientWindow *window);

    /*!
     * \brief An application window was unmapped.
     * \param window Application window.
     */
    void windowUnmapped(ClientWindow *window);

private:
    ApplicationManagerPrivate *const d_ptr;

    friend class ClientWindow;
};

}

#endif // APPLICATIONMANAGER_H
