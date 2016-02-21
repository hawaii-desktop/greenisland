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

#ifndef GREENISLAND_GTKSHELL_H
#define GREENISLAND_GTKSHELL_H

#include <GreenIsland/server/greenislandserver_export.h>
#include <GreenIsland/QtWaylandCompositor/QWaylandExtension>

class QWaylandClient;
class QWaylandSurface;

namespace GreenIsland {

namespace Server {

class GtkShellPrivate;
class GtkSurfacePrivate;

class GREENISLANDSERVER_EXPORT GtkShell : public QWaylandExtensionTemplate<GtkShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GtkShell)
public:
    GtkShell();
    GtkShell(QWaylandCompositor *compositor);

    void initialize() Q_DECL_OVERRIDE;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void createSurface(QWaylandSurface *surface,
                       QWaylandClient *client, uint id);
};

class GREENISLANDSERVER_EXPORT GtkSurface : public QWaylandExtensionTemplate<GtkSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GtkSurface)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
public:
    GtkSurface();
    GtkSurface(GtkShell *shell, QWaylandSurface *surface,
               QWaylandClient *client, uint id);

    Q_INVOKABLE void initialize(GtkShell *shell, QWaylandSurface *surface,
                                QWaylandClient *client, uint id);

    QWaylandSurface *surface() const;

    QString appId() const;
    QString appMenuPath() const;
    QString menuBarPath() const;
    QString windowObjectPath() const;
    QString appObjectPath() const;
    QString uniqueBusName() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void surfaceChanged();

    void appIdChanged(const QString &appId);
    void appMenuPathChanged(const QString &appMenuPath);
    void menuBarPathChanged(const QString &menuBarPath);
    void windowObjectPathChanged(const QString &windowObjectPath);
    void appObjectPathChanged(const QString &appObjectPath);
    void uniqueBusNameChanged(const QString &uniqueBusName);

    void setModal();
    void unsetModal();

private:
    void initialize();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_GTKSHELL_H
