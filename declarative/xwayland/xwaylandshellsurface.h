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

#ifndef XWAYLANDWINDOW_H
#define XWAYLANDWINDOW_H

#include <QtCore/QRect>
#include <QtCore/QPointer>

#include <GreenIsland/QtWaylandCompositor/QWaylandSurface>

#include <xcb/xcb.h>

#include "xcbwindow.h"
#include "sizehints.h"

class XWaylandManager;
class XWaylandSurface;
class XWaylandQuickShellIntegration;
class XWaylandQuickShellSurfaceItem;

class XWaylandShellSurface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(int x READ x NOTIFY xChanged)
    Q_PROPERTY(int y READ y NOTIFY yChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(bool activated READ activated NOTIFY activatedChanged)
    Q_PROPERTY(bool maximized READ maximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
public:
    enum ResizeEdge {
        NoneEdge        =  0,
        TopEdge         =  1,
        BottomEdge      =  2,
        LeftEdge        =  4,
        TopLeftEdge     =  5,
        BottomLeftEdge  =  6,
        RightEdge       =  8,
        TopRightEdge    =  9,
        BottomRightEdge = 10
    };
    Q_ENUM(ResizeEdge);

    enum WmState {
        WithdrawnState = 0,
        NormalState = 1,
        IconicState = 3
    };

    XWaylandShellSurface(xcb_window_t window, const QRect &geometry,
                         bool overrideRedirect, XWaylandManager *parent);
    ~XWaylandShellSurface();

    inline bool isMaximized() const {
        return m_properties.maximizedHorizontally || m_properties.maximizedVertically;
    }

    quint32 surfaceId() const;
    void setSurfaceId(quint32 id);

    QWaylandSurface *surface() const;
    void setSurface(QWaylandSurface *surface);

    int x() const;
    void setX(int x);

    int y() const;
    void setY(int y);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    bool activated() const;

    bool maximized() const;
    void setMaximized(bool maximized);

    bool fullscreen() const;
    void setFullscreen(bool fullscreen);

    inline WmState wmState() const {
        return m_wmState;
    }

    void setWmState(WmState state);

    void setNetWmState();

    inline int workspace() const {
        return m_workspace;
    }

    void setWorkspace(int workspace);

    void readProperties();
    void setProperties();

    void readAndDumpProperty(xcb_atom_t atom);

    void requestResize(const QSize &size);
    void resizeFrame(const QSize &size, quint32 mask, quint32 *values);

    QSize sizeForResize(const QSizeF &initialSize, const QPointF &delta, ResizeEdge edges);
    void sendConfigure(const QSize &size);

    void map();
    void unmap();

    XWaylandQuickShellIntegration *createIntegration(XWaylandQuickShellSurfaceItem *item);

    operator xcb_window_t() const;
    xcb_window_t window() const;

Q_SIGNALS:
    void surfaceChanged();
    void surfaceDestroyed();
    void mapped();
    void unmapped();
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();
    void activatedChanged();
    void maximizedChanged();
    void fullscreenChanged();
    void startMove();
    void startResize(ResizeEdge edges);

private:
    XWaylandManager *m_wm;
    xcb_window_t m_window;
    QRect m_geometry;
    bool m_propsDirty;
    bool m_overrideRedirect;
    bool m_hasAlpha;
    XWaylandShellSurface *m_transientFor;
    quint32 m_surfaceId;
    QWaylandSurface *m_surface;
    WmState m_wmState;
    int m_workspace;
    WmSizeHints m_sizeHints;
    MotifWmHints m_motifHints;

    struct {
        QString title;
        QString appId;
        QPoint pos;
        QSize size;
        QSize savedSize;
        int fullscreen;
        int maximizedHorizontally;
        int maximizedVertically;
        int deleteWindow;
    } m_properties;

    int m_x;
    int m_y;
    int m_width;
    int m_height;

    bool m_activated;
    bool m_maximized;
    bool m_fullscreen;

    void dumpProperty(xcb_atom_t property, xcb_get_property_reply_t *reply);
    void *decodeProperty(xcb_atom_t type, xcb_get_property_reply_t *reply);

    friend class XWaylandManager;

private Q_SLOTS:
    void handleSurfaceDestroyed();
};

#endif // XWAYLANDWINDOW_H
