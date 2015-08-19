/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "inputdevice.h"
#include "inputpanel.h"
#include "output.h"
#include "region.h"

#include "qwlcompositor_p.h"
#include "qwldisplay_p.h"
#include "qwlsurface_p.h"
#include "clientconnection.h"
#include "abstractcompositor.h"
#include "qwldatadevicemanager_p.h"
#include "qwldatadevice_p.h"
#include "qwlextendedsurface_p.h"
#include "qwlsubsurface_p.h"
#include "qwlqtkey_p.h"
#include "qwlinputdevice_p.h"
#include "qwlpointer_p.h"
#include "qwltextinputmanager_p.h"
#include "globalinterface.h"
#include "surfaceview.h"
#include "shmformathelper_p.h"
#include "qwlkeyboard_p.h"

#include <QWindow>
#include <QSocketNotifier>
#include <QScreen>
#include <qpa/qplatformscreen.h>
#include <QGuiApplication>
#include <QDebug>

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/private/qguiapplication_p.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>

#include <wayland-server.h>

#if defined (QT_COMPOSITOR_WAYLAND_GL)
#include "protocols/qt/qthardwareintegration.h"
#include "plugins/clientbufferintegrationfactory_p.h"
#include "plugins/serverbufferintegrationfactory_p.h"
#endif
#include "protocols/qt/qttouchextension.h"
#include "protocols/qt/qtwindowmanager.h"

#include "xkbhelper_p.h"

using namespace GreenIsland;

namespace GreenIsland {

static WlCompositor *compositor;

class WindowSystemEventHandler : public QWindowSystemEventHandler
{
public:
    WindowSystemEventHandler(WlCompositor *c) : compositor(c) {}
    bool sendEvent(QWindowSystemInterfacePrivate::WindowSystemEvent *e) Q_DECL_OVERRIDE
    {
        if (e->type == QWindowSystemInterfacePrivate::Key) {
            QWindowSystemInterfacePrivate::KeyEvent *ke = static_cast<QWindowSystemInterfacePrivate::KeyEvent *>(e);
            WlKeyboard *keyb = compositor->defaultInputDevice()->keyboardDevice();

            uint32_t code = ke->nativeScanCode;
            bool isDown = ke->keyType == QEvent::KeyPress;

#ifdef HAVE_XKBCOMMON
            QString text;
            Qt::KeyboardModifiers modifiers = XkbHelper::modifiers(keyb->xkbState());

            const xkb_keysym_t sym = xkb_state_key_get_one_sym(keyb->xkbState(), code);
            uint utf32 = xkb_keysym_to_utf32(sym);
            if (utf32)
                text = QString::fromUcs4(&utf32, 1);
            int qtkey = XkbHelper::keysymToQtKey(sym, modifiers, text);

            ke->key = qtkey;
            ke->modifiers = modifiers;
            ke->nativeVirtualKey = sym;
            ke->nativeModifiers = keyb->xkbModsMask();
            ke->unicode = text;
#endif
            if (!ke->repeat)
                keyb->keyEvent(code, isDown ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);

            QWindowSystemEventHandler::sendEvent(e);

            if (!ke->repeat) {
                keyb->updateKeymap();
                keyb->updateModifierState(code, isDown ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);
            }
        } else {
            QWindowSystemEventHandler::sendEvent(e);
        }
        return true;
    }

    WlCompositor *compositor;
};

WlCompositor *WlCompositor::instance()
{
    return compositor;
}

WlCompositor::WlCompositor(AbstractCompositor *qt_compositor, AbstractCompositor::ExtensionFlags extensions)
    : m_extensions(extensions)
    , m_display(new Display)
    , m_current_frame(0)
    , m_last_queued_buf(-1)
    , m_qt_compositor(qt_compositor)
    , m_orientation(Qt::PrimaryOrientation)
#if defined (QT_COMPOSITOR_WAYLAND_GL)
    , m_hw_integration(0)
    , m_client_buffer_integration(0)
    , m_server_buffer_integration(0)
#endif
    , m_windowManagerIntegration(0)
    , m_surfaceExtension(0)
    , m_touchExtension(0)
    , m_qtkeyExtension(0)
    , m_textInputManager()
    , m_inputPanel()
    , m_eventHandler(new WindowSystemEventHandler(this))
    , m_retainSelection(false)
{
    m_timer.start();
    compositor = this;

    QWindowSystemInterfacePrivate::installWindowSystemEventHandler(m_eventHandler.data());
}

void WlCompositor::init()
{
    QStringList arguments = QCoreApplication::instance()->arguments();

    int socketArg = arguments.indexOf(QLatin1String("--wayland-socket-name"));
    if (socketArg != -1 && socketArg + 1 < arguments.size())
        m_socket_name = arguments.at(socketArg + 1).toLocal8Bit();

    wl_compositor::init(m_display->handle(), 3);
    wl_subcompositor::init(m_display->handle(), 1);

    m_data_device_manager =  new WlDataDeviceManager(this);

    wl_display_init_shm(m_display->handle());
    QVector<wl_shm_format> formats = ShmFormatHelper::supportedWaylandFormats();
    foreach (wl_shm_format format, formats)
        wl_display_add_shm_format(m_display->handle(), format);

    if (wl_display_add_socket(m_display->handle(), m_qt_compositor->socketName())) {
        fprintf(stderr, "Fatal: Failed to open server socket\n");
        exit(EXIT_FAILURE);
    }

    m_loop = wl_display_get_event_loop(m_display->handle());

    int fd = wl_event_loop_get_fd(m_loop);

    QSocketNotifier *sockNot = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(sockNot, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));

    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    connect(dispatcher, SIGNAL(aboutToBlock()), this, SLOT(processWaylandEvents()));

    //initialize distancefieldglyphcache here

    initializeHardwareIntegration();
    initializeExtensions();
    initializeDefaultInputDevice();
}

WlCompositor::~WlCompositor()
{
    if (!m_destroyed_surfaces.isEmpty())
        qWarning("Compositor::cleanupGraphicsResources() must be called manually");
    qDeleteAll(m_clients);

    qDeleteAll(m_outputs);

    delete m_surfaceExtension;
    delete m_touchExtension;
    delete m_qtkeyExtension;

    removeInputDevice(m_default_wayland_input_device);
    delete m_default_wayland_input_device;
    delete m_data_device_manager;

    delete m_display;
}

void WlCompositor::sendFrameCallbacks(QList<Surface *> visibleSurfaces)
{
    foreach (Surface *surface, visibleSurfaces) {
        surface->handle()->sendFrameCallback();
    }
    wl_display_flush_clients(m_display->handle());
}

uint WlCompositor::currentTimeMsecs() const
{
    return m_timer.elapsed();
}

QList<Output *> WlCompositor::outputs() const
{
    return m_outputs;
}

Output *WlCompositor::output(OutputWindow *window) const
{
    Q_FOREACH (Output *output, m_outputs) {
        if (output->window() == window)
            return output;
    }

    return Q_NULLPTR;
}

void WlCompositor::addOutput(Output *output)
{
    if (m_outputs.contains(output))
        return;

    m_outputs.append(output);
}

void WlCompositor::removeOutput(Output *output)
{
    m_outputs.removeOne(output);
}

Output *WlCompositor::primaryOutput() const
{
    if (m_outputs.size() == 0)
        return Q_NULLPTR;
    return m_outputs.at(0);
}

void WlCompositor::setPrimaryOutput(Output *output)
{
    int i = m_outputs.indexOf(output);
    if (i <= 0)
        return;

    m_outputs.removeAt(i);
    m_outputs.prepend(output);
}

void WlCompositor::processWaylandEvents()
{
    int ret = wl_event_loop_dispatch(m_loop, 0);
    if (ret)
        fprintf(stderr, "wl_event_loop_dispatch error: %d\n", ret);
    wl_display_flush_clients(m_display->handle());
}

void WlCompositor::destroySurface(WlSurface *surface)
{
    m_surfaces.removeOne(surface);

    waylandCompositor()->surfaceAboutToBeDestroyed(surface->waylandSurface());

    surface->releaseSurfaces();
    m_destroyed_surfaces << surface->waylandSurface();
}

void WlCompositor::resetInputDevice(WlSurface *surface)
{
    foreach (InputDevice *dev, m_inputDevices) {
        if (dev->keyboardFocus() == surface->waylandSurface())
            dev->setKeyboardFocus(0);
        if (dev->mouseFocus() && dev->mouseFocus()->surface() == surface->waylandSurface())
            dev->setMouseFocus(0, QPointF(), QPointF());
    }
}

void WlCompositor::cleanupGraphicsResources()
{
    qDeleteAll(m_destroyed_surfaces);
    m_destroyed_surfaces.clear();
}

void WlCompositor::compositor_create_surface(wl_compositor::Resource *resource, uint32_t id)
{
    Surface *surface = new Surface(resource->client(), id, resource->version(), m_qt_compositor);
    m_surfaces << surface->handle();
    surface->handle()->addToOutput(primaryOutput());
    //BUG: This may not be an on-screen window surface though
    m_qt_compositor->surfaceCreated(surface);
}

void WlCompositor::compositor_create_region(wl_compositor::Resource *resource, uint32_t id)
{
    Q_UNUSED(compositor);
    new Region(resource->client(), id);
}

void WlCompositor::destroyClient(ClientConnection *client)
{
    if (!client)
        return;

    if (m_windowManagerIntegration)
        m_windowManagerIntegration->sendQuitMessage(client->client());

    wl_client_destroy(client->client());
}

void WlCompositor::subcompositor_destroy(wl_subcompositor::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WlCompositor::subcompositor_get_subsurface(wl_subcompositor::Resource *resource, uint32_t id, wl_resource *surface, wl_resource *parent)
{
    WlSurface *s = WlSurface::fromResource(surface);
    WlSurface *p = WlSurface::fromResource(parent);

    static const char where[] = "get_subsurface: wl_subsurface@";
    if (s == p) {
        wl_resource_post_error(resource->handle, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "%s%d: wl_surface@%d cannot be its own parent", where, id, wl_resource_get_id(surface));
        return;
    }
    if (SubSurface::get(s)) {
        wl_resource_post_error(resource->handle, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "%s%d: wl_surface@%d is already a sub-surface", where, id, wl_resource_get_id(surface));
        return;
    }

    if (!s->setRole(SubSurface::role(), resource->handle, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE))
        return;

    SubSurface *ss = new SubSurface(s, p, resource->client(), id, resource->version());
    s->setRoleHandler(ss);
}

ClientBufferIntegrationInterface * WlCompositor::clientBufferIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    return m_client_buffer_integration.data();
#else
    return 0;
#endif
}

ServerBufferIntegrationInterface *WlCompositor::serverBufferIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    return m_server_buffer_integration.data();
#else
    return 0;
#endif
}

void WlCompositor::initializeHardwareIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (m_extensions & AbstractCompositor::HardwareIntegrationExtension)
        m_hw_integration.reset(new QtHardwareIntegration(m_display->handle()));

    loadClientBufferIntegration();
    loadServerBufferIntegration();

    if (m_client_buffer_integration)
        m_client_buffer_integration->initializeHardware(m_display);
    if (m_server_buffer_integration)
        m_server_buffer_integration->initializeHardware(m_qt_compositor);
#endif
}

void WlCompositor::initializeExtensions()
{
    if (m_extensions & AbstractCompositor::SurfaceExtension)
        m_surfaceExtension = new WlSurfaceExtensionGlobal(this);
    if (m_extensions & AbstractCompositor::TouchExtension)
        m_touchExtension = new QtTouchExtension(this);
    if (m_extensions & AbstractCompositor::QtKeyExtension)
        m_qtkeyExtension = new QtKeyExtensionGlobal(this);
    if (m_extensions & AbstractCompositor::TextInputExtension) {
        m_textInputManager.reset(new WlTextInputManager(this));
        m_inputPanel.reset(new InputPanel);
    }
    if (m_extensions & AbstractCompositor::WindowManagerExtension) {
        m_windowManagerIntegration = new QtWindowManager(m_qt_compositor, this);
        m_windowManagerIntegration->initialize(m_display);
    }
}

void WlCompositor::initializeDefaultInputDevice()
{
    m_default_wayland_input_device = new InputDevice(m_qt_compositor);
    registerInputDevice(m_default_wayland_input_device);
}

QList<ClientConnection *> WlCompositor::clients() const
{
    return m_clients;
}

void WlCompositor::setClientFullScreenHint(bool value)
{
    if (m_windowManagerIntegration)
        m_windowManagerIntegration->setShowIsFullScreen(value);
}

AbstractCompositor::ExtensionFlags WlCompositor::extensions() const
{
    return m_extensions;
}

WlInputDevice* WlCompositor::defaultInputDevice()
{
    // The list gets prepended so that default is the last element
    return m_inputDevices.last()->handle();
}

void WlCompositor::configureTouchExtension(int flags)
{
    if (m_touchExtension)
        m_touchExtension->setFlags(flags);
}

InputPanel *WlCompositor::inputPanel() const
{
    return m_inputPanel.data();
}

WlDataDeviceManager *WlCompositor::dataDeviceManager() const
{
    return m_data_device_manager;
}

void WlCompositor::setRetainedSelectionEnabled(bool enabled)
{
    m_retainSelection = enabled;
}

bool WlCompositor::retainedSelectionEnabled() const
{
    return m_retainSelection;
}

void WlCompositor::feedRetainedSelectionData(QMimeData *data)
{
    if (m_retainSelection)
        m_qt_compositor->retainedSelectionReceived(data);
}

void WlCompositor::overrideSelection(const QMimeData *data)
{
    m_data_device_manager->overrideSelection(*data);
}

bool WlCompositor::isDragging() const
{
    return false;
}

void WlCompositor::sendDragMoveEvent(const QPoint &global, const QPoint &local,
                                            WlSurface *surface)
{
    Q_UNUSED(global);
    Q_UNUSED(local);
    Q_UNUSED(surface);
//    Drag::instance()->dragMove(global, local, surface);
}

void WlCompositor::sendDragEndEvent()
{
//    Drag::instance()->dragEnd();
}

void WlCompositor::bindGlobal(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    GreenIsland::GlobalInterface *iface = static_cast<GreenIsland::GlobalInterface *>(data);
    iface->bind(client, qMin(iface->version(), version), id);
};

void WlCompositor::loadClientBufferIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    ClientBufferIntegrationInterface *integration = ClientBufferIntegrationFactory::loadPlugin();
    if (integration) {
        m_client_buffer_integration.reset(integration);
        if (m_client_buffer_integration) {
            m_client_buffer_integration->setCompositor(m_qt_compositor);
            if (m_hw_integration)
                m_hw_integration->setClientBufferIntegration(m_client_buffer_integration->name());
        }
    }
    //BUG: if there is no client buffer integration, bad things will happen when opengl is used
#endif
}

void WlCompositor::loadServerBufferIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    ServerBufferIntegrationInterface *integration = ServerBufferIntegrationFactory::loadPlugin();
    if (integration) {
        m_server_buffer_integration.reset(integration);
        if (m_hw_integration)
            m_hw_integration->setServerBufferIntegration(m_server_buffer_integration->name());
    }
#endif
}

void WlCompositor::registerInputDevice(InputDevice *device)
{
    // The devices get prepended as the first input device that gets added
    // is assumed to be the default and it will claim to accept all the input
    // events if asked
    m_inputDevices.prepend(device);
}

void WlCompositor::removeInputDevice(InputDevice *device)
{
    m_inputDevices.removeOne(device);
}

InputDevice *WlCompositor::inputDeviceFor(QInputEvent *inputEvent)
{
    InputDevice *dev = NULL;
    for (int i = 0; i < m_inputDevices.size(); i++) {
        InputDevice *candidate = m_inputDevices.at(i);
        if (candidate->isOwner(inputEvent)) {
            dev = candidate;
            break;
        }
    }
    return dev;
}

} // namespace Wayland

