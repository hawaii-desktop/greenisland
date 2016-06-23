/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini
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

#include <QtQuick/QQuickWindow>
#include <QDebug>

#include "keyeventfilter.h"

KeyEventFilter::KeyEventFilter(QQuickItem *parent)
    : QQuickItem(parent)
{
    connect(this, &QQuickItem::windowChanged, this, [=](QQuickWindow *window) {
        // Remove event filter previously installed if any
        if (!m_window.isNull()) {
            m_window->removeEventFilter(this);
            m_window.clear();
        }

        // Install this event filter when the item is on the window
        if (window) {
            window->installEventFilter(this);
            m_window = window;
        }
    });
}

bool KeyEventFilter::eventFilter(QObject *object, QEvent *event)
{
    // Only filter the window we are attached to
    if (object != m_window.data())
        return QObject::eventFilter(object, event);

    // Discard events not related to keyboard
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return false;

    // Pass this event to QML for processing, but do not eat it so it
    // will still be delivered to the currently focused application window
    event->accept();
    QCoreApplication::sendEvent(this, event);
    return event->isAccepted();
}

#include "moc_keyeventfilter.cpp"
