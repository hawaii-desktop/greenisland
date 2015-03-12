/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2013-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <QtCompositor/QWaylandSurfaceOp>

#include <greenisland/greenisland_export.h>

class QWaylandOutput;
class QWaylandSurface;
class QWaylandSurfaceItem;

namespace GreenIsland {

class Compositor;
class Output;
class WlShellSurface;
class XdgPopup;
class XdgSurface;
class ScreenManagerPrivate;
class WindowView;

class GREENISLAND_EXPORT ClientWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint id READ id CONSTANT)
    Q_PROPERTY(QWaylandSurface *surface READ surface CONSTANT)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(ClientWindow *parentWindow READ parentWindow CONSTANT)
    Q_PROPERTY(QWaylandOutput *output READ output NOTIFY outputChanged)
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY positionChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY positionChanged)
    Q_PROPERTY(QPointF position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QSizeF size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QRectF geometry READ geometry NOTIFY geometryChanged)
    Q_PROPERTY(QRectF internalGeometry READ internalGeometry NOTIFY internalGeometryChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(bool minimized READ isMinimized NOTIFY minimizedChanged)
    Q_PROPERTY(bool maximized READ isMaximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullScreen READ isFullScreen NOTIFY fullScreenChanged)
    Q_ENUMS(Type)
public:
    enum Type {
        TopLevel = 0,
        Popup,
        Transient
    };

    enum CustomSurfaceOperation {
        Move = QWaylandSurfaceOp::UserType + 1
    };

    ClientWindow(QWaylandSurface *surface, QObject *parent = 0);
    ~ClientWindow();

    uint id() const;

    QWaylandSurface *surface() const;

    Type type() const;
    QString title() const;
    QString appId() const;
    QString iconName() const;

    ClientWindow *parentWindow() const;

    QWaylandOutput *output() const;

    Q_INVOKABLE QWaylandSurfaceItem *viewForOutput(Output *output);

    qreal x() const;
    void setX(qreal value);

    qreal y() const;
    void setY(qreal value);

    QPointF position() const;
    void setPosition(const QPointF &pos);

    QSizeF size() const;
    QRectF geometry() const;
    QRectF internalGeometry() const;

    bool isActive() const;
    Q_INVOKABLE void activate();
    Q_INVOKABLE void deactivate();

    bool isMinimized() const;
    Q_INVOKABLE void minimize();
    Q_INVOKABLE void unminimize();

    bool isMaximized() const;
    Q_INVOKABLE void maximize();
    Q_INVOKABLE void unmaximize();

    bool isFullScreen() const;
    void setFullScreen(bool fs);

    Q_INVOKABLE void move();

Q_SIGNALS:
    void typeChanged();
    void titleChanged();
    void appIdChanged();
    void iconNameChanged();
    void outputChanged();
    void positionChanged();
    void sizeChanged();
    void geometryChanged();
    void internalGeometryChanged();
    void activeChanged();
    void minimizedChanged();
    void maximizedChanged();
    void fullScreenChanged();
    void windowMenuRequested(const QPoint &pt);
    void motionStarted();
    void motionFinished();

private:
    Type m_type;
    QString m_appId;
    QString m_iconName;
    QPointF m_pos;
    QSizeF m_size;
    QRectF m_internalGeometry;
    bool m_internalGeometryChanged;
    bool m_active;
    bool m_minimized;
    bool m_maximized;
    bool m_fullScreen;
    bool m_initialSetup;
    Compositor *m_compositor;
    QWaylandSurface *m_surface;
    ClientWindow *m_parentWindow;
    QHash<Output *, WindowView *> m_views;

    void setSize(const QSizeF &size);
    void setInternalGeometry(const QRectF &geometry);

    void registerWindow();
    void unregisterWindow(bool destruction);

    QPointF calculateInitialPosition() const;

    void initialSetup();
    void removeOutput(Output *output);

    QVariant readFromDesktopFile(const QString &baseName, const QString &key,
                                 const QVariant &defaultValue = QVariant()) const;

    friend class WlShellSurface;
    friend class XdgPopup;
    friend class XdgSurface;
    friend class ScreenManagerPrivate;

    static uint m_id;

private Q_SLOTS:
    void setType(QWaylandSurface::WindowType windowType);
};

} // namespace GreenIsland

#endif // CLIENTWINDOW_H
