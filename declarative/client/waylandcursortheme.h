/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef WAYLANDCURSORTHEME_H
#define WAYLANDCURSORTHEME_H

#include <QtCore/QObject>

#include <GreenIsland/Client/Compositor>
#include <GreenIsland/Client/CursorTheme>
#include <GreenIsland/Client/Registry>
#include <GreenIsland/Client/Seat>
#include <GreenIsland/Client/ShmPool>

using namespace GreenIsland::Client;

class WaylandConnection;

class WaylandCursorTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(WaylandConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(CursorShape shape READ shape WRITE setShape NOTIFY shapeChanged)
public:
    enum CursorShape {
        ArrowCursor = Qt::ArrowCursor,
        UpArrowCursor,
        CrossCursor,
        WaitCursor,
        IBeamCursor,
        SizeVerCursor,
        SizeHorCursor,
        SizeBDiagCursor,
        SizeFDiagCursor,
        SizeAllCursor,
        BlankCursor,
        SplitVCursor,
        SplitHCursor,
        PointingHandCursor,
        ForbiddenCursor,
        WhatsThisCursor,
        BusyCursor,
        OpenHandCursor,
        ClosedHandCursor,
        DragCopyCursor,
        DragMoveCursor,
        DragLinkCursor,
        ResizeNorthCursor = Qt::CustomCursor + 1,
        ResizeSouthCursor,
        ResizeEastCursor,
        ResizeWestCursor,
        ResizeNorthWestCursor,
        ResizeSouthEastCursor,
        ResizeNorthEastCursor,
        ResizeSouthWestCursor
    };
    Q_ENUM(CursorShape)

    WaylandCursorTheme(QObject *parent = nullptr);
    ~WaylandCursorTheme();

    WaylandConnection *connection() const;
    void setConnection(WaylandConnection *connection);

    CursorShape shape() const;
    void setShape(CursorShape shape);

Q_SIGNALS:
    void connectionChanged();
    void shapeChanged();

private:
    bool m_initialized;
    WaylandConnection *m_connection;
    Registry *m_registry;
    Compositor *m_compositor;
    Shm *m_shm;
    ShmPool *m_shmPool;
    QVector<Output *> m_outputs;
    int m_outputsDone;
    Seat *m_seat;
    CursorTheme *m_cursorTheme;
    CursorShape m_cursorShape;

private Q_SLOTS:
    void handleConnected();

    void interfaceAnnounced(const QByteArray &interface, quint32 name, quint32 version);
    void interfacesAnnounced();
    void interfacesRemoved();
};

#endif // WAYLANDCURSORTHEME_H
