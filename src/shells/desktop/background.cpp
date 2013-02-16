/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (C) 2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QDebug>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QScreen>

#include "background.h"

Background::Background(QScreen *screen, QObject *parent)
    : QObject(parent)
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(QUrl("qrc:///qml/Background.qml"));
    if (!component.isReady())
        qFatal(qPrintable(component.errorString()));

    QObject *topLevel = component.create();
    m_window = qobject_cast<QQuickWindow *>(topLevel);
    if (!m_window)
        qFatal("Error: Background root item must be a Window!\n");
    m_window->setScreen(screen);

    // This is a frameless window that stays on top of everything
    m_window->setFlags(Qt::CustomWindow);

    // Set the title
    m_window->setTitle(QString("Background on %1").arg(screen->name()));

    // Create the platform window
    m_window->winId();

    // Set screen size and detect geometry changes
    updateScreenGeometry();
    connect(screen, SIGNAL(geometryChanged(QRect)),
            this, SLOT(updateScreenGeometry(QRect)));
}

void Background::updateScreenGeometry()
{
    updateScreenGeometry(m_window->screen()->geometry());
}

void Background::updateScreenGeometry(const QRect &geometry)
{
    m_window->setGeometry(geometry);
}

#include "moc_background.cpp"
