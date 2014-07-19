/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include "qscreenbackend.h"

QScreenBackend::QScreenBackend(QObject *parent)
    : ScreenBackend(parent)
{
    connect(qApp, SIGNAL(screenAdded(QScreen*)),
            this, SLOT(slotScreenAdded(QScreen*)));
    initialize();
}

QScreenBackend::~QScreenBackend()
{
    qDeleteAll(m_screens);
}

int QScreenBackend::count() const
{
    return m_screens.size();
}

Screen *QScreenBackend::screenAt(int index)
{
    return m_screens.at(index);
}

void QScreenBackend::initialize()
{
    for (QScreen *qscreen: QGuiApplication::screens())
        slotScreenAdded(qscreen);
}

void QScreenBackend::slotGeometryChanged(const QRect &rect)
{
    QScreen *qscreen = qobject_cast<QScreen *>(sender());

    for (Screen *screen: m_screens) {
        if (screen->name() == qscreen->name()) {
            screen->setGeometry(rect);
            Q_EMIT screenChangedGeometry();
            return;
        }
    }
}

void QScreenBackend::slotScreenAdded(QScreen *qscreen)
{
    Screen *screen = new Screen(this);
    screen->setName(qscreen->name());
    screen->setPrimary(QGuiApplication::primaryScreen() == qscreen);
    screen->setGeometry(qscreen->geometry());

    connect(qscreen, SIGNAL(geometryChanged(QRect)),
            this, SLOT(slotGeometryChanged(QRect)));
    connect(qscreen, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotScreenDestroyed(QObject*)));

    Q_EMIT screenAdded();
}

void QScreenBackend::slotScreenDestroyed(QObject *object)
{
    QScreen *qscreen = qobject_cast<QScreen *>(object);

    Screen *screen = Q_NULLPTR;
    for (int i = 0; i < m_screens.size(); i++) {
        screen = m_screens.at(i);
        if (screen->name() == qscreen->name())
            break;
    }

    if (screen) {
        m_screens.removeAll(screen);
        screen->deleteLater();

        Q_EMIT screenRemoved();
    }

    // Reassign primary role
    for (int i = 0; i < m_screens.size(); i++) {
        screen = m_screens.at(i);

        bool primary = screen->name() == QGuiApplication::primaryScreen()->name();
        screen->setPrimary(primary);
    }
}

#include "moc_qscreenbackend.cpp"
