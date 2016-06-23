/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2016 Pier Luigi Fiorini
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

#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H

#include <QtCore/QTimer>
#include <QtQuick/QQuickItem>

class FpsCounter : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(unsigned int fps READ fps NOTIFY fpsChanged)
public:
    explicit FpsCounter(QQuickItem *parent = 0);

    unsigned int fps() const;

Q_SIGNALS:
    void fpsChanged();

private Q_SLOTS:
    void setup();
    void frameSwapped();
    void updateFps();

private:
    unsigned int m_counter;
    unsigned int m_fps;
    QTimer m_timer;
};

#endif // FPSCOUNTER_H
