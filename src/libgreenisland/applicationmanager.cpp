/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "applicationmanager.h"
#include "applicationmanager_p.h"

namespace GreenIsland {

/*
 * ApplicationManagerPrivate
 */

ApplicationManagerPrivate::ApplicationManagerPrivate(ApplicationManager *parent)
    : q_ptr(parent)
{
}

void ApplicationManagerPrivate::registerApplication(const QString &appId)
{
    Q_Q(ApplicationManager);

    if (apps.contains(appId))
        return;

    Q_EMIT q->applicationAdded(appId);
}

void ApplicationManagerPrivate::unregisterApplication(const QString &appId)
{
    Q_Q(ApplicationManager);

    if (apps.remove(appId))
        Q_EMIT q->applicationRemoved(appId);
}

/*
 * ApplicationManager
 */

ApplicationManager::ApplicationManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new ApplicationManagerPrivate(this))
{
    qRegisterMetaType<ApplicationManager *>("ApplicationManager*");
}

ApplicationManager::~ApplicationManager()
{
    delete d_ptr;
}

bool ApplicationManager::isRegistered(const QString &appId) const
{
    Q_D(const ApplicationManager);
    return d->apps.contains(appId);
}

}

#include "moc_applicationmanager.cpp"
