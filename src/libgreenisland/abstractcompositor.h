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

#ifndef GREENISLAND_ABSTRACTCOMPOSITOR_H
#define GREENISLAND_ABSTRACTCOMPOSITOR_H

#include <greenisland/greenisland_export.h>

#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtGui/QImage>

struct wl_display;

class QInputEvent;
class QMimeData;
class QOpenGLContext;
class QUrl;

namespace GreenIsland {

class ClientConnection;
class Drag;
class GlobalInterface;
class InputDevice;
class InputPanel;
class Output;
class OutputWindow;
class Surface;
class SurfaceView;

class WlCompositor;

class GREENISLAND_EXPORT AbstractCompositor
{
public:
    enum ExtensionFlag {
        WindowManagerExtension = 0x01,
        SurfaceExtension = 0x02,
        QtKeyExtension = 0x04,
        TouchExtension = 0x08,
        SubSurfaceExtension = 0x10,
        TextInputExtension = 0x20,
        HardwareIntegrationExtension = 0x40,

        DefaultExtensions = WindowManagerExtension | SurfaceExtension | QtKeyExtension | TouchExtension | HardwareIntegrationExtension
    };
    Q_DECLARE_FLAGS(ExtensionFlags, ExtensionFlag)

    AbstractCompositor(const char *socketName = 0, ExtensionFlags extensions = DefaultExtensions);
    virtual ~AbstractCompositor();

    void addGlobalInterface(GlobalInterface *interface);
    void addDefaultShell();
    ::wl_display *waylandDisplay() const;

    void frameStarted();
    void sendFrameCallbacks(QList<Surface *> visibleSurfaces);

    void destroyClientForSurface(Surface *surface);
    void destroyClient(ClientConnection *client);

    QList<Surface *> surfacesForClient(ClientConnection* client) const;
    QList<Surface *> surfaces() const;

    QList<Output *> outputs() const;
    Output *output(OutputWindow *window);

    Output *primaryOutput() const;
    void setPrimaryOutput(Output *output);

    virtual void surfaceCreated(Surface *surface) = 0;
    virtual void surfaceAboutToBeDestroyed(Surface *surface);

    virtual SurfaceView *pickView(const QPointF &globalPosition) const;
    virtual QPointF mapToView(SurfaceView *view, const QPointF &surfacePosition) const;

    virtual bool openUrl(ClientConnection *client, const QUrl &url);

    WlCompositor *handle() const;

    void setRetainedSelectionEnabled(bool enabled);
    bool retainedSelectionEnabled() const;
    void overrideSelection(const QMimeData *data);

    void setClientFullScreenHint(bool value);

    const char *socketName() const;

    InputDevice *defaultInputDevice() const;

    InputPanel *inputPanel() const;
    Drag *drag() const;

    bool isDragging() const;
    void sendDragMoveEvent(const QPoint &global, const QPoint &local, Surface *surface);
    void sendDragEndEvent();

    virtual void setCursorSurface(Surface *surface, int hotspotX, int hotspotY);

    void cleanupGraphicsResources();

    enum TouchExtensionFlag {
        TouchExtMouseFromTouch = 0x01
    };
    Q_DECLARE_FLAGS(TouchExtensionFlags, TouchExtensionFlag)
    void configureTouchExtension(TouchExtensionFlags flags);

    virtual SurfaceView *createView(Surface *surface);

    InputDevice *inputDeviceFor(QInputEvent *inputEvent);

protected:
    AbstractCompositor(const char *socketName, WlCompositor *dptr);
    virtual void retainedSelectionReceived(QMimeData *mimeData);

    friend class WlCompositor;
    WlCompositor *m_compositor;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractCompositor::ExtensionFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractCompositor::TouchExtensionFlags)

} // namespace GreenIsland

#endif // GREENISLAND_ABSTRACTCOMPOSITOR_H
