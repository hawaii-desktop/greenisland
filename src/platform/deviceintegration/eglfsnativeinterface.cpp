/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include "egldeviceintegration.h"
#include "eglfscontext.h"
#include "eglfsintegration.h"
#include "eglfsnativeinterface.h"
#include "eglfsscreen.h"
#include "eglfswindow.h"

namespace GreenIsland {

namespace Platform {

enum ResourceType {
    EglDisplay,
    EglWindow,
    EglContext,
    EglConfig,
    NativeDisplay,
    XlibDisplay,
    WaylandDisplay
};

static int resourceType(const QByteArray &key)
{
    static const QByteArray names[] = { // match ResourceType
                                        QByteArrayLiteral("egldisplay"),
                                        QByteArrayLiteral("eglwindow"),
                                        QByteArrayLiteral("eglcontext"),
                                        QByteArrayLiteral("eglconfig"),
                                        QByteArrayLiteral("nativedisplay"),
                                        QByteArrayLiteral("display"),
                                        QByteArrayLiteral("server_wl_display")
                                      };
    const QByteArray *end = names + sizeof(names) / sizeof(names[0]);
    const QByteArray *result = std::find(names, end, key);
    if (result == end)
        result = std::find(names, end, key.toLower());
    return int(result - names);
}

EglFSNativeInterface::EglFSNativeInterface(EglFSIntegration *integration)
    : QPlatformNativeInterface()
    , m_integration(integration)
{
}

void *EglFSNativeInterface::nativeResourceForIntegration(const QByteArray &resource)
{
    void *result = 0;

    switch (resourceType(resource)) {
    case EglDisplay:
        result = m_integration->display();
        break;
    case NativeDisplay:
        result = reinterpret_cast<void*>(m_integration->nativeDisplay());
        break;
    case WaylandDisplay:
        result = egl_device_integration()->wlDisplay();
        break;
    default:
        break;
    }

    return result;
}

void *EglFSNativeInterface::nativeResourceForScreen(const QByteArray &resource, QScreen *)
{
    void *result = 0;

    switch (resourceType(resource)) {
    case XlibDisplay:
        // Play nice when using the x11 hooks: Be compatible with xcb that allows querying
        // the X Display pointer, which is nothing but our native display.
        result = reinterpret_cast<void*>(m_integration->nativeDisplay());
        break;
    default:
        break;
    }

    return result;
}

void *EglFSNativeInterface::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
    void *result = 0;

    switch (resourceType(resource)) {
    case EglDisplay:
        if (window && window->handle())
            result = static_cast<EglFSScreen *>(window->handle()->screen())->display();
        else
            result = m_integration->display();
        break;
    case EglWindow:
        if (window && window->handle())
            result = reinterpret_cast<void*>(static_cast<EglFSWindow *>(window->handle())->eglWindow());
        break;
    default:
        break;
    }

    return result;
}

void *EglFSNativeInterface::nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context)
{
    void *result = 0;

    switch (resourceType(resource)) {
    case EglContext:
        if (context->handle())
            result = static_cast<EglFSContext *>(context->handle())->eglContext();
        break;
    case EglConfig:
        if (context->handle())
            result = static_cast<EglFSContext *>(context->handle())->eglConfig();
        break;
    case EglDisplay:
        if (context->handle())
            result = static_cast<EglFSContext *>(context->handle())->eglDisplay();
        break;
    default:
        break;
    }

    return result;
}

static void *eglContextForContext(QOpenGLContext *context)
{
    Q_ASSERT(context);

    EglFSContext *handle = static_cast<EglFSContext *>(context->handle());
    if (!handle)
        return 0;

    return handle->eglContext();
}

QPlatformNativeInterface::NativeResourceForContextFunction EglFSNativeInterface::nativeResourceFunctionForContext(const QByteArray &resource)
{
    QByteArray lowerCaseResource = resource.toLower();
    if (lowerCaseResource == "get_egl_context")
        return NativeResourceForContextFunction(eglContextForContext);

    return 0;
}

QFunctionPointer EglFSNativeInterface::platformFunction(const QByteArray &function) const
{
    Q_UNUSED(function);
    return 0;
}

} // namespace Platform

} // namespace GreenIsland
