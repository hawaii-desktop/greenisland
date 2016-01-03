/****************************************************************************
 * This file is part of Hawaii.
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

#ifndef XWAYLANDWINDOW_H
#define XWAYLANDWINDOW_H

#include <QtCore/QRect>
#include <QtCore/QPointer>
#include <GreenIsland/Compositor/QWaylandSurface>

#include <xcb/xcb.h>

#include "xcbwindow.h"
#include "sizehints.h"

namespace GreenIsland {

class ClientWindow;
class XWaylandManager;
class XWaylandSurface;

class XWaylandWindow : public QObject
{
    Q_OBJECT
public:
    enum WmState {
        WithdrawnState = 0,
        NormalState = 1,
        IconicState = 3
    };

    XWaylandWindow(xcb_window_t window, const QRect &geometry,
                   bool overrideRedirect, XWaylandManager *parent);
    ~XWaylandWindow();

    inline bool isDecorated() const {
        return m_decorated;
    }

    inline bool isMaximized() const {
        return m_properties.maximizedHorizontally || m_properties.maximizedVertically;
    }

    inline xcb_window_t frameId() const {
        return m_frameWindow.window();
    }

    inline void setFrameId(xcb_window_t id) {
        m_frameWindow = Xcb::Window(id);
    }

    inline int surfaceId() const {
        return m_surfaceId;
    }

    inline void setSurfaceId(int id) {
        m_surfaceId = id;
    }

    inline QWaylandSurface *surface() const {
        return m_surface.data();
    }

    void setSurface(QWaylandSurface *surface);

    ClientWindow *clientWindow() const;

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

    void map();
    void unmap();

    void repaint();

    operator xcb_window_t() const;
    xcb_window_t window() const;

private:
    XWaylandManager *m_wm;
    xcb_window_t m_window;
    QRect m_geometry;
    bool m_propsDirty;
    bool m_overrideRedirect;
    bool m_hasAlpha;
    bool m_decorated;
    Xcb::Window m_frameWindow;
    XWaylandWindow *m_transientFor;
    int m_surfaceId;
    QPointer<QWaylandSurface> m_surface;
    XWaylandSurface *m_surfaceInterface;
    WmState m_wmState;
    int m_workspace;
    WmSizeHints m_sizeHints;
    MotifWmHints m_motifHints;

    struct WindowProperties {
        QString title;
        QString appId;
        QPoint pos;
        QSize size;
        QSize savedSize;
        int fullscreen;
        int maximizedHorizontally;
        int maximizedVertically;
        int deleteWindow;
    };

    WindowProperties m_properties;

    void dumpProperty(xcb_atom_t property, xcb_get_property_reply_t *reply);
    void *decodeProperty(xcb_atom_t type, xcb_get_property_reply_t *reply);

    friend class XWaylandManager;

private Q_SLOTS:
    void surfaceDestroyed();
};

}

#endif // XWAYLANDWINDOW_H
