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

#ifndef GLOBALPOINTERTRACKER_H
#define GLOBALPOINTERTRACKER_H

#include <QtCore/QObject>

class QWaylandCompositor;
class QWaylandOutput;

class GlobalPointerTrackerPrivate;

class GlobalPointerTracker : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GlobalPointerTracker)
    Q_PROPERTY(QWaylandCompositor *compositor READ compositor WRITE setCompositor NOTIFY compositorChanged)
    Q_PROPERTY(bool enableSystemPointer READ isSystemPointerEnabled WRITE setSystemPointerEnabled NOTIFY enableSystemPointerChanged)
    Q_PROPERTY(qreal mouseX READ mouseX WRITE setMouseX NOTIFY xChanged)
    Q_PROPERTY(qreal mouseY READ mouseY WRITE setMouseY NOTIFY yChanged)
    Q_PROPERTY(QWaylandOutput *output READ output NOTIFY outputChanged)
public:
    GlobalPointerTracker(QObject *parent = Q_NULLPTR);

    QWaylandCompositor *compositor() const;
    void setCompositor(QWaylandCompositor *compositor);

    bool isSystemPointerEnabled() const;
    void setSystemPointerEnabled(bool enable);

    qreal mouseX() const;
    void setMouseX(qreal mouseX);

    qreal mouseY() const;
    void setMouseY(qreal mouseY);

    QWaylandOutput *output() const;

Q_SIGNALS:
    void compositorChanged();
    void enableSystemPointerChanged();
    void xChanged();
    void yChanged();
    void outputChanged();
};

#endif // GLOBALPOINTERTRACKER_H
