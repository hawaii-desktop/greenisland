/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLANDCOMPOSITOR_H
#define GREENISLANDCOMPOSITOR_H

#include <QtCore/QLoggingCategory>
#include <QtCore/QProcess>
#include <QtQuick/QQuickView>
#include <QtCompositor/QWaylandCompositor>
#include <QtCompositor/QWaylandSurface>

#include <GreenIsland/GreenIslandGlobal>

Q_DECLARE_LOGGING_CATEGORY(GREENISLAND_COMPOSITOR)

namespace GreenIsland {

class CompositorPrivate;

class GREENISLAND_EXPORT Compositor : public QQuickView, public QWaylandCompositor
{
    Q_OBJECT
    Q_PROPERTY(QString shellFileName READ shellFileName WRITE setShellFileName NOTIFY shellFileNameChanged)
    Q_DECLARE_PRIVATE(Compositor)
public:
    explicit Compositor(const char *socketName = 0,
                        QWaylandCompositor::ExtensionFlag extensions = QWaylandCompositor::DefaultExtensions);
    ~Compositor();

    QString shellFileName() const;
    void setShellFileName(const QString &fileName);

    void showGraphicsInfo();

    void runShell(const QStringList &arguments = QStringList());
    void closeShell();

    virtual void surfaceCreated(QWaylandSurface *surface);
    virtual void surfaceAboutToBeDestroyed(QWaylandSurface *surface);

Q_SIGNALS:
    void shellFileNameChanged(const QString &fileName);
    void ready();

    void surfaceMapped(QWaylandSurface *surface);
    void surfaceUnmapped(QWaylandSurface *surface);
    void surfaceDestroyed(QWaylandSurface *surface);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_shellStarted())
    Q_PRIVATE_SLOT(d_func(), void _q_shellFailed(QProcess::ProcessError error))
    Q_PRIVATE_SLOT(d_func(), void _q_shellReadyReadStandardOutput())
    Q_PRIVATE_SLOT(d_func(), void _q_shellReadyReadStandardError())
    Q_PRIVATE_SLOT(d_func(), void _q_shellAboutToClose())

    CompositorPrivate *const d_ptr;
};

}

#endif // GREENISLANDCOMPOSITOR_H
