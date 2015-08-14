/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WL_COMPOSITOR_H
#define WL_COMPOSITOR_H

#include <greenisland/greenisland_export.h>
#include "abstractcompositor.h"

#include "qwayland-server-wayland.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QSet>

#include "wayland_wrapper/qwldisplay_p.h"

#include <wayland-server.h>

class QInputEvent;
class QMimeData;
class QPlatformScreenBuffer;

class QWindowSystemEventHandler;

namespace GreenIsland {

class AbstractCompositor;
class ClientConnection;
class ClientConnectionPrivate;
class InputDevice;
class InputPanel;
class GlobalInterface;
class Surface;
class SurfaceBuffer;

class WlSurface;
class WlInputDevice;
class WlDataDeviceManager;
class OutputGlobal;
class WlSurfaceExtensionGlobal;
class SubSurfaceExtensionGlobal;
class QtTouchExtension;
class QtKeyExtensionGlobal;
class WlTextInputManager;
class HardwareIntegration;
class ClientBufferIntegration;
class ServerBufferIntegration;
class QtWindowManager;

class GREENISLAND_EXPORT WlCompositor : public QObject, public QtWaylandServer::wl_compositor, public QtWaylandServer::wl_subcompositor
{
    Q_OBJECT

public:
    WlCompositor(AbstractCompositor *qt_compositor, AbstractCompositor::ExtensionFlags extensions);
    ~WlCompositor();

    void init();
    void sendFrameCallbacks(QList<Surface *> visibleSurfaces);

    WlInputDevice *defaultInputDevice();

    void registerInputDevice(InputDevice *device);
    QList<InputDevice *> inputDevices() const { return m_inputDevices; }
    InputDevice *inputDeviceFor(QInputEvent *inputEvent);
    void removeInputDevice(InputDevice *device);

    void destroySurface(WlSurface *surface);

    void destroyClient(ClientConnection *client);

    uint currentTimeMsecs() const;

    QList<AbstractOutput *> outputs() const;
    AbstractOutput *output(QWindow *window) const;

    void addOutput(AbstractOutput *output);
    void removeOutput(AbstractOutput *output);

    AbstractOutput *primaryOutput() const;
    void setPrimaryOutput(AbstractOutput *output);

    ClientBufferIntegration *clientBufferIntegration() const;
    ServerBufferIntegration *serverBufferIntegration() const;
    void initializeHardwareIntegration();
    void initializeExtensions();
    void initializeDefaultInputDevice();
    void initializeWindowManagerProtocol();

    QList<WlSurface *> surfaces() const { return m_surfaces; }

    AbstractCompositor *waylandCompositor() const { return m_qt_compositor; }

    struct wl_display *wl_display() const { return m_display->handle(); }
    Display *display() const { return m_display; }

    static WlCompositor *instance();

    QList<ClientConnection *> clients() const;

    QtWindowManager *windowManagerIntegration() const { return m_windowManagerIntegration; }

    void setClientFullScreenHint(bool value);

    AbstractCompositor::ExtensionFlags extensions() const;

    QtTouchExtension *touchExtension() { return m_touchExtension; }
    void configureTouchExtension(int flags);

    QtKeyExtensionGlobal *qtkeyExtension() { return m_qtkeyExtension; }

    InputPanel *inputPanel() const;

    WlDataDeviceManager *dataDeviceManager() const;

    bool isDragging() const;
    void sendDragMoveEvent(const QPoint &global, const QPoint &local, WlSurface *surface);
    void sendDragEndEvent();

    void setRetainedSelectionEnabled(bool enabled);
    bool retainedSelectionEnabled() const;
    void overrideSelection(const QMimeData *data);
    void feedRetainedSelectionData(QMimeData *data);

    static void bindGlobal(wl_client *client, void *data, uint32_t version, uint32_t id);
    void resetInputDevice(WlSurface *surface);

public slots:
    void cleanupGraphicsResources();

protected:
    void compositor_create_surface(wl_compositor::Resource *resource, uint32_t id) Q_DECL_OVERRIDE;
    void compositor_create_region(wl_compositor::Resource *resource, uint32_t id) Q_DECL_OVERRIDE;
    void subcompositor_destroy(wl_subcompositor::Resource *resource) Q_DECL_OVERRIDE;
    void subcompositor_get_subsurface(wl_subcompositor::Resource *resource, uint32_t id, wl_resource *surface, wl_resource *parent) Q_DECL_OVERRIDE;

private slots:
    void processWaylandEvents();

protected:
    void loadClientBufferIntegration();
    void loadServerBufferIntegration();

    AbstractCompositor::ExtensionFlags m_extensions;

    Display *m_display;
    QByteArray m_socket_name;

    /* Input */
    InputDevice *m_default_wayland_input_device;

    QList<InputDevice *> m_inputDevices;

    /* Output */
    QList<AbstractOutput *> m_outputs;

    WlDataDeviceManager *m_data_device_manager;

    QElapsedTimer m_timer;
    QList<WlSurface *> m_surfaces;
    QSet<Surface *> m_destroyed_surfaces;

    /* Render state */
    uint32_t m_current_frame;
    int m_last_queued_buf;

    wl_event_loop *m_loop;

    AbstractCompositor *m_qt_compositor;
    Qt::ScreenOrientation m_orientation;
    QList<ClientConnection *> m_clients;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    QScopedPointer<HardwareIntegration> m_hw_integration;
    QScopedPointer<ClientBufferIntegration> m_client_buffer_integration;
    QScopedPointer<ServerBufferIntegration> m_server_buffer_integration;
#endif

    //extensions
    QtWindowManager *m_windowManagerIntegration;

    WlSurfaceExtensionGlobal *m_surfaceExtension;
    QtTouchExtension *m_touchExtension;
    QtKeyExtensionGlobal *m_qtkeyExtension;
    QScopedPointer<WlTextInputManager> m_textInputManager;
    QScopedPointer<InputPanel> m_inputPanel;
    QList<GreenIsland::GlobalInterface *> m_globals;
    QScopedPointer<QWindowSystemEventHandler> m_eventHandler;

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);

    bool m_retainSelection;

    friend class AbstractCompositor;
    friend class ClientConnection;
    friend class ClientConnectionPrivate;
};

}

#endif //WL_COMPOSITOR_H
