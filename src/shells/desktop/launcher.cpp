/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QScreen>
#include <QTimer>

#include <VibeCore/VSettings>

#include "launcher.h"

Launcher::Launcher(QScreen *screen, QObject *parent)
    : QObject(parent)
{
    // Settings
    m_settings = new VSettings(QStringLiteral("org.hawaii.greenisland"));

    // Engine
    m_engine = new QQmlEngine(this);
    m_engine->rootContext()->setContextProperty(
                QStringLiteral("launcherObject"), this);

    // Load component
    m_component = new QQmlComponent(m_engine, this);
    m_component->loadUrl(QUrl("qrc:///qml/Launcher.qml"));
    if (!m_component->isReady())
        qFatal(qPrintable(m_component->errorString()));

    // Create component
    QObject *topLevel = m_component->create();
    m_window = qobject_cast<QQuickWindow *>(topLevel);
    if (!m_window)
        qFatal("Error: Launcher root item must be a Window!\n");
    m_window->setScreen(screen);

    // This is a frameless window that stays on top of everything
    m_window->setFlags(Qt::WindowStaysOnTopHint | Qt::CustomWindow);

    // Create the platform window and set geometry
    m_window->create();
    m_window->setGeometry(geometry());

    // When screen geometry changes our geometry changes as well and we
    // emit a signal
    connect(screen, SIGNAL(geometryChanged(QRect)),
            this, SIGNAL(geometryChanged(QRect)));
}

Launcher::~Launcher()
{
    delete m_window;
    delete m_component;
    delete m_engine;
    delete m_settings;
}

QRect Launcher::geometry() const
{
    QString alignment = m_settings->value("launcher/alignment").toString();
    QRect screenGeometry = m_window->screen()->availableGeometry();
    QPoint pt(0, 0);
    QSize size;

    if (alignment == QStringLiteral("bottom")) {
        pt.setY(screenGeometry.height() - windowSize());
        size = QSize(screenGeometry.width(), windowSize());
    } else
        size = QSize(windowSize(), screenGeometry.height());

    return QRect(pt, size);
}

int Launcher::windowSize() const
{
    return m_window->property("size").toInt();
}

int Launcher::tileSize() const
{
    return m_window->property("tileSize").toInt();
}

#include "moc_launcher.cpp"
