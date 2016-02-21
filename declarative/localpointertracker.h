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

#ifndef LOCALPOINTERTRACKER_P
#define LOCALPOINTERTRACKER_P

#include <QtQuick/QQuickItem>

class GlobalPointerTracker;
class LocalPointerTrackerPrivate;

class LocalPointerTracker : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LocalPointerTracker)
    Q_PROPERTY(GlobalPointerTracker *globalTracker READ globalTracker WRITE setGlobalTracker NOTIFY globalTrackerChanged)
    Q_PROPERTY(qreal mouseX READ mouseX NOTIFY mouseXChanged)
    Q_PROPERTY(qreal mouseY READ mouseY NOTIFY mouseYChanged)
public:
    LocalPointerTracker(QQuickItem *parent = Q_NULLPTR);

    GlobalPointerTracker *globalTracker() const;
    void setGlobalTracker(GlobalPointerTracker *tracker);

    qreal mouseX() const;
    qreal mouseY() const;

Q_SIGNALS:
    void globalTrackerChanged();
    void mouseXChanged();
    void mouseYChanged();

protected:
    bool childMouseEventFilter(QQuickItem *item, QEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void hoverMoveEvent(QHoverEvent *event) Q_DECL_OVERRIDE;
};

#endif // LOCALPOINTERTRACKER_P
