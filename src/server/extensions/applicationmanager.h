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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositorExtension>

#include <GreenIsland/server/greenislandserver_export.h>

class QWaylandCompositor;
class QWaylandOutput;
class QWaylandSurface;

namespace GreenIsland {

namespace Server {

class ApplicationManagerPrivate;
class ClientWindow;

class GREENISLANDSERVER_EXPORT ApplicationManager : public QWaylandCompositorExtensionTemplate<ApplicationManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ApplicationManager)
    Q_PROPERTY(QWaylandCompositor *compositor READ compositor NOTIFY compositorChanged)
    Q_PROPERTY(GreenIsland::Server::ClientWindow *focusedWindow READ focusedWindow NOTIFY focusedWindowChanged)
    Q_PRIVATE_PROPERTY(ApplicationManager::d_func(), QQmlListProperty<GreenIsland::Server::ClientWindow> windows READ windows NOTIFY windowsChanged)
public:
    ApplicationManager();
    ApplicationManager(QWaylandCompositor *compositor);

    QWaylandCompositor *compositor() const;

    /*!
     * Returns the focused window.
     */
    GreenIsland::Server::ClientWindow *focusedWindow() const;

    Q_INVOKABLE GreenIsland::Server::ClientWindow *createWindow(QWaylandSurface *surface);

    Q_INVOKABLE GreenIsland::Server::ClientWindow *windowForSurface(QWaylandSurface *surface) const;
    Q_INVOKABLE QVariantList windowsForOutput(QWaylandOutput *desiredOutput = Q_NULLPTR) const;

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
     * \brief A compositor was associated with this extension.
     */
    void compositorChanged();

    /*!
     * \brief Focus had been given to another window.
     */
    void focusedWindowChanged();

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
     * \brief A new window was created.
     * \param window Window that was just created.
     */
    void windowCreated(GreenIsland::Server::ClientWindow *window);

    /*!
     * \brief A window was closed.
     * \param window Window that was closed.
     */
    void windowClosed(GreenIsland::Server::ClientWindow *window);

    /*!
     * \brief A window was either created or closed.
     *
     * Creating and closing windows affects the windows list,
     * when that happens this signal is emitted.
     */
    void windowsChanged();

private:
    Q_PRIVATE_SLOT(d_func(), void _q_appIdChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_activatedChanged())
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_APPLICATIONMANAGER_H
