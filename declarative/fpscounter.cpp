/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
