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
