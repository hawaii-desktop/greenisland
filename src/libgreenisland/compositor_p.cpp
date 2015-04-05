/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifdef QT_COMPOSITOR_WAYLAND_GL
#  include "bufferattacher.h"
#endif
#include "compositor.h"
#include "compositor_p.h"
#include "clientwindow.h"
#include "logging.h"
#include "shellwindow.h"

namespace GreenIsland {

CompositorPrivate::CompositorPrivate(Compositor *self)
    : running(false)
    , state(Compositor::Active)
    , idleInterval(5 * 60000)
    , idleInhibit(0)
    , locked(false)
    , cursorSurface(Q_NULLPTR)
    , cursorHotspotX(0)
    , cursorHotspotY(0)
    , lastKeyboardFocus(Q_NULLPTR)
    , recorderManager(Q_NULLPTR)
    , q_ptr(self)
{
    settings = new CompositorSettings(self);
    screenManager = new ScreenManager(self);

    ApplicationManager::instance();
}

CompositorPrivate::~CompositorPrivate()
{
    screenManager->deleteLater();
}

QQmlListProperty<ClientWindow> CompositorPrivate::windows()
{
    Q_Q(Compositor);

    auto countFunc = [](QQmlListProperty<ClientWindow> *prop) {
        return static_cast<Compositor *>(prop->object)->d_func()->clientWindowsList.count();
    };
    auto atFunc = [](QQmlListProperty<ClientWindow> *prop, int index) {
        return static_cast<Compositor *>(prop->object)->d_func()->clientWindowsList.at(index);
    };
    return QQmlListProperty<ClientWindow>(q, 0, countFunc, atFunc);
}

QQmlListProperty<ShellWindow> CompositorPrivate::shellWindows()
{
    Q_Q(Compositor);

    auto countFunc = [](QQmlListProperty<ShellWindow> *prop) {
        return static_cast<Compositor *>(prop->object)->d_func()->shellWindowsList.count();
    };
    auto atFunc = [](QQmlListProperty<ShellWindow> *prop, int index) {
        return static_cast<Compositor *>(prop->object)->d_func()->shellWindowsList.at(index);
    };
    return QQmlListProperty<ShellWindow>(q, 0, countFunc, atFunc);
}

void CompositorPrivate::dpms(bool on)
{
    // TODO
    Q_UNUSED(on);
}

void CompositorPrivate::_q_updateCursor(bool hasBuffer)
{
    if (!hasBuffer || !cursorSurface || !cursorSurface->bufferAttacher())
        return;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    QImage image = static_cast<BufferAttacher *>(cursorSurface->bufferAttacher())->image();
    QCursor cursor(QPixmap::fromImage(image), cursorHotspotX, cursorHotspotY);

    static bool cursorIsSet = false;
    if (cursorIsSet) {
        QGuiApplication::changeOverrideCursor(cursor);
    } else {
        QGuiApplication::setOverrideCursor(cursor);
        cursorIsSet = true;
    }
#endif
}

void CompositorPrivate::addWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    if (!clientWindowsList.contains(window)) {
        clientWindowsList.append(window);
        Q_EMIT q->windowsChanged();
    }
}

void CompositorPrivate::removeWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    if (clientWindowsList.removeOne(window))
        Q_EMIT q->windowsChanged();
}

void CompositorPrivate::mapWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    addWindow(window);

    Q_EMIT ApplicationManager::instance()->windowMapped(window);
    Q_EMIT q->windowMapped(QVariant::fromValue(window));
}

void CompositorPrivate::unmapWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    removeWindow(window);

    Q_EMIT ApplicationManager::instance()->windowUnmapped(window);
    Q_EMIT q->windowUnmapped(QVariant::fromValue(window));
}

void CompositorPrivate::destroyWindow(ClientWindow *window)
{
    Q_Q(Compositor);

    removeWindow(window);

    Q_EMIT ApplicationManager::instance()->windowUnmapped(window);
    Q_EMIT q->windowDestroyed(window->id());
}

void CompositorPrivate::mapShellWindow(ShellWindow *window)
{
    Q_Q(Compositor);

    if (!shellWindowsList.contains(window)) {
        shellWindowsList.append(window);
        Q_EMIT q->shellWindowMapped(QVariant::fromValue(window));
        Q_EMIT q->shellWindowsChanged();
    }
}

void CompositorPrivate::unmapShellWindow(ShellWindow *window)
{
    Q_Q(Compositor);

    if (shellWindowsList.removeOne(window)) {
        Q_EMIT q->shellWindowUnmapped(QVariant::fromValue(window));
        Q_EMIT q->shellWindowsChanged();
    }
}

void CompositorPrivate::destroyShellWindow(ShellWindow *window)
{
    Q_Q(Compositor);

    if (shellWindowsList.removeOne(window)) {
        Q_EMIT q->shellWindowDestroyed(window->id());
        Q_EMIT q->shellWindowsChanged();
    }
}

}
