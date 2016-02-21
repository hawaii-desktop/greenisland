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

#ifndef GREENISLAND_APPLICATIONMANAGER_H
#define GREENISLAND_APPLICATIONMANAGER_H

#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>

#include <GreenIsland/server/greenislandserver_export.h>

class QWaylandCompositor;

namespace GreenIsland {

namespace Server {

class ApplicationManagerPrivate;

class GREENISLANDSERVER_EXPORT ApplicationManager : public QWaylandExtensionTemplate<ApplicationManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ApplicationManager)
public:
    ApplicationManager();
    ApplicationManager(QWaylandCompositor *compositor);

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

    /*!
     * \brief Initializes the extension.
     */
    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

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
    Q_PRIVATE_SLOT(d_func(), void _q_appIdChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_activeChanged())
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_APPLICATIONMANAGER_H
