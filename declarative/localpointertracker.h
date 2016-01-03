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
