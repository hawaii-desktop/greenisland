/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
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

#ifndef DESKTOPSHELL_H
#define DESKTOPSHELL_H

#include <QQuickView>

#include <VShell>

class DesktopShell : public QQuickView, public VShell
{
    Q_OBJECT
    Q_PROPERTY(QRectF availableGeometry READ availableGeometry WRITE setAvailableGeometry NOTIFY availableGeometryChanged)
    Q_PROPERTY(WaylandSurface *currentSurface READ currentSurface WRITE setCurrentSurface NOTIFY currentSurfaceChanged)
public:
    DesktopShell();

    QRectF availableGeometry() const;
    void setAvailableGeometry(const QRectF &rect);

    WaylandSurface *currentSurface() const {
        return m_currentSurface;
    }

    void setupCompositor(bool fullscreen);
    void startShell();

    void surfaceCreated(WaylandSurface *surface);
    void surfaceAboutToBeDestroyed(WaylandSurface *surface);

    Q_INVOKABLE void disconnectUser();

signals:
    void windowAdded(QVariant window);
    void windowDestroyed(QVariant window);
    void windowResized(QVariant window);
    void availableGeometryChanged();
    void currentSurfaceChanged();

public slots:
    void destroyWindow(QVariant window);
    void destroyClientForWindow(QVariant window);

    void setCurrentSurface(WaylandSurface *surface);

private slots:
    void surfaceMapped();
    void surfaceUnmapped();
    void surfaceDestroyed(QObject *object);
    void frameSwapped();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QRectF m_availableGeometry;
    WaylandSurface *m_currentSurface;
};

#endif // DESKTOPSHELL_H
