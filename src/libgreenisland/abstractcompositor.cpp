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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright (C) 2015 The Qt Company Ltd.
 *
 *     You may use this file under the terms of the BSD license as follows:
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions are
 *     met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in
 *         the documentation and/or other materials provided with the
 *         distribution.
 *       * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 *         of its contributors may be used to endorse or promote products derived
 *         from this software without specific prior written permission.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ***************************************************************************/

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtGui/QDesktopServices>
#include <QtGui/QScreen>

#include "abstractcompositor.h"
#include "clientconnection.h"
#include "globalinterface.h"
#include "inputdevice.h"
#include "inputpanel.h"
#include "output.h"
#include "surfaceview.h"

#include "wayland_wrapper/qwlcompositor_p.h"
#include "wayland_wrapper/qwldatadevice_p.h"
#include "wayland_wrapper/qwlsurface_p.h"
#include "wayland_wrapper/qwlinputdevice_p.h"

namespace GreenIsland {

AbstractCompositor::AbstractCompositor(const char *socketName, ExtensionFlags extensions)
    : m_compositor(new WlCompositor(this, extensions))
{
    m_compositor->m_socket_name = socketName;
    m_compositor->init();
}

AbstractCompositor::AbstractCompositor(const char *socketName, GreenIsland::WlCompositor *dptr)
    : m_compositor(dptr)
{
    m_compositor->m_socket_name = socketName;
    m_compositor->init();
}

AbstractCompositor::~AbstractCompositor()
{
    qDeleteAll(m_compositor->m_globals);
    delete m_compositor;
}

void AbstractCompositor::addGlobalInterface(GlobalInterface *interface)
{
    wl_global_create(m_compositor->wl_display(), interface->interface(), interface->version(), interface, WlCompositor::bindGlobal);
    m_compositor->m_globals << interface;
}

void AbstractCompositor::addDefaultShell()
{
}

struct wl_display *AbstractCompositor::waylandDisplay() const
{
    return m_compositor->wl_display();
}

void AbstractCompositor::sendFrameCallbacks(QList<Surface *> visibleSurfaces)
{
    m_compositor->sendFrameCallbacks(visibleSurfaces);
}

void AbstractCompositor::frameStarted()
{
    foreach (WlSurface *surf, m_compositor->surfaces())
        surf->frameStarted();
}

void AbstractCompositor::destroyClientForSurface(Surface *surface)
{
    destroyClient(surface->client());
}

void AbstractCompositor::destroyClient(ClientConnection *client)
{
    m_compositor->destroyClient(client);
}

QList<Surface *> AbstractCompositor::surfacesForClient(ClientConnection* client) const
{
    QList<WlSurface *> surfaces = m_compositor->surfaces();

    QList<Surface *> result;

    for (int i = 0; i < surfaces.count(); ++i) {
        if (surfaces.at(i)->waylandSurface()->client() == client) {
            result.append(surfaces.at(i)->waylandSurface());
        }
    }

    return result;
}

QList<Surface *> AbstractCompositor::surfaces() const
{
    QList<WlSurface *> surfaces = m_compositor->surfaces();
    QList<Surface *> surfs;
    surfs.reserve(surfaces.count());
    foreach (WlSurface *s, surfaces)
        surfs << s->waylandSurface();
    return surfs;
}

QList<Output *> AbstractCompositor::outputs() const
{
    return m_compositor->outputs();
}

Output *AbstractCompositor::output(OutputWindow *window)
{
    return m_compositor->output(window);
}

Output *AbstractCompositor::primaryOutput() const
{
    return m_compositor->primaryOutput();
}

void AbstractCompositor::setPrimaryOutput(Output *output)
{
    m_compositor->setPrimaryOutput(output);
}

void AbstractCompositor::cleanupGraphicsResources()
{
    m_compositor->cleanupGraphicsResources();
}

void AbstractCompositor::surfaceAboutToBeDestroyed(Surface *surface)
{
    Q_UNUSED(surface);
}

SurfaceView *AbstractCompositor::pickView(const QPointF &globalPosition) const
{
    Q_FOREACH (Output *output, outputs()) {
        // Skip coordinates not in output
        if (!QRectF(output->geometry()).contains(globalPosition))
            continue;

        Q_FOREACH (Surface *surface, output->surfaces()) {
            Q_FOREACH (SurfaceView *view, surface->views()) {
                if (QRectF(view->pos(), surface->size()).contains(globalPosition))
                    return view;
            }
        }
    }

    return Q_NULLPTR;
}

QPointF AbstractCompositor::mapToView(SurfaceView *surface, const QPointF &globalPosition) const
{
    return globalPosition - surface->pos();
}

/*!
    Override this to handle QDesktopServices::openUrl() requests from the clients.

    The default implementation simply forwards the request to QDesktopServices::openUrl().
*/
bool AbstractCompositor::openUrl(ClientConnection *client, const QUrl &url)
{
    Q_UNUSED(client);
    return QDesktopServices::openUrl(url);
}

WlCompositor *AbstractCompositor::handle() const
{
    return m_compositor;
}

void AbstractCompositor::setRetainedSelectionEnabled(bool enabled)
{
    m_compositor->setRetainedSelectionEnabled(enabled);
}

bool AbstractCompositor::retainedSelectionEnabled() const
{
    return m_compositor->retainedSelectionEnabled();
}

void AbstractCompositor::retainedSelectionReceived(QMimeData *)
{
}

void AbstractCompositor::overrideSelection(const QMimeData *data)
{
    m_compositor->overrideSelection(data);
}

void AbstractCompositor::setClientFullScreenHint(bool value)
{
    m_compositor->setClientFullScreenHint(value);
}

const char *AbstractCompositor::socketName() const
{
    if (m_compositor->m_socket_name.isEmpty())
        return 0;
    return m_compositor->m_socket_name.constData();
}

InputDevice *AbstractCompositor::defaultInputDevice() const
{
    return m_compositor->defaultInputDevice()->handle();
}

InputPanel *AbstractCompositor::inputPanel() const
{
    return m_compositor->inputPanel();
}

Drag *AbstractCompositor::drag() const
{
    return m_compositor->defaultInputDevice()->dragHandle();
}

bool AbstractCompositor::isDragging() const
{
    return m_compositor->isDragging();
}

void AbstractCompositor::sendDragMoveEvent(const QPoint &global, const QPoint &local,
                                           Surface *surface)
{
    m_compositor->sendDragMoveEvent(global, local, surface ? surface->handle() : 0);
}

void AbstractCompositor::sendDragEndEvent()
{
    m_compositor->sendDragEndEvent();
}

void AbstractCompositor::setCursorSurface(Surface *surface, int hotspotX, int hotspotY)
{
    Q_UNUSED(surface);
    Q_UNUSED(hotspotX);
    Q_UNUSED(hotspotY);
}

void AbstractCompositor::configureTouchExtension(TouchExtensionFlags flags)
{
    m_compositor->configureTouchExtension(flags);
}

SurfaceView *AbstractCompositor::createView(Surface *surface)
{
    return new SurfaceView(surface);
}

InputDevice *AbstractCompositor::inputDeviceFor(QInputEvent *inputEvent)
{
    return m_compositor->inputDeviceFor(inputEvent);
}

} // namespace GreenIsland
