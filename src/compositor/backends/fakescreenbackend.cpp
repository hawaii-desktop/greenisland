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

#include <QtCore/QRect>

#include "fakescreenbackend.h"
#include "compositorapp.h"

FakeScreenBackend::FakeScreenBackend(QObject *parent)
    : ScreenBackend(parent)
{
    CompositorApp *app = qobject_cast<CompositorApp *>(qApp);

    QPoint pt(0, 0);
    for (int i = 0; i < count(); i++) {
        Screen *screen = new Screen(this);
        screen->setName(QString("FakeScreen%1").arg(i));
        screen->setPrimary(i == 0);
        screen->setGeometry(QRect(pt, app->fakeScreenSize()));
        pt.setX(pt.x() + app->fakeScreenSize().width());
        m_screens.append(screen);
    }
}

FakeScreenBackend::~FakeScreenBackend()
{
    qDeleteAll(m_screens);
}

int FakeScreenBackend::count() const
{
    CompositorApp *app = qobject_cast<CompositorApp *>(qApp);
    return app->fakeScreenCount();
}

Screen *FakeScreenBackend::screenAt(int index)
{
    return m_screens.at(index);
}

#include "moc_fakescreenbackend.cpp"
