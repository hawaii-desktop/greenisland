/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#include <QtQuick/QQuickWindow>

#include "fpscounter.h"

FpsCounter::FpsCounter(QQuickItem *parent)
    : QQuickItem(parent)
    , m_counter(0)
    , m_fps(0)
{
    setFlag(QQuickItem::ItemHasContents, false);
    QMetaObject::invokeMethod(this, "setup", Qt::QueuedConnection);
}

unsigned int FpsCounter::fps() const
{
    return m_fps;
}

void FpsCounter::setup()
{
    QQuickWindow *parentWindow = window();
    connect(parentWindow, SIGNAL(frameSwapped()),
            this, SLOT(frameSwapped()));

    connect(&m_timer, SIGNAL(timeout()),
            this, SLOT(updateFps()));
    m_timer.setInterval(2000);
    m_timer.start();
}

void FpsCounter::frameSwapped()
{
    m_counter++;
}

void FpsCounter::updateFps()
{
    if (m_counter > 0) {
        m_fps = m_counter / 2;
        m_counter = 0;
        Q_EMIT fpsChanged();
    }
}

#include "moc_fpscounter.cpp"
