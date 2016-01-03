/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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

#include "eglstreamconvenience.h"
#include "logging.h"

#include <string.h>

namespace GreenIsland {

namespace Platform {

EGLStreamConvenience::EGLStreamConvenience()
    : initialized(false),
      has_egl_platform_device(false),
      has_egl_device_base(false),
      has_egl_stream(false),
      has_egl_stream_producer_eglsurface(false),
      has_egl_stream_consumer_egloutput(false),
      has_egl_output_drm(false),
      has_egl_output_base(false),
      has_egl_stream_cross_process_fd(false),
      has_egl_stream_consumer_gltexture(false)
{
    const char *extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!extensions) {
        qCWarning(lcEglConvenience, "Failed to query EGL extensions");
        return;
    }

    query_devices = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(eglGetProcAddress("eglQueryDevicesEXT"));
    query_device_string = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(eglGetProcAddress("eglQueryDeviceStringEXT"));
    get_platform_display = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));

    has_egl_device_base = strstr(extensions, "EGL_EXT_device_base");
    has_egl_platform_device = strstr(extensions, "EGL_EXT_platform_device");
}

void EGLStreamConvenience::initialize(EGLDisplay dpy)
{
    if (initialized)
        return;

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        qCWarning(lcEglConvenience, "Failed to bind OpenGL ES API");
        return;
    }

    const char *extensions = eglQueryString(dpy, EGL_EXTENSIONS);
    if (!extensions) {
        qCWarning(lcEglConvenience, "Failed to query EGL extensions");
        return;
    }

    create_stream = reinterpret_cast<PFNEGLCREATESTREAMKHRPROC>(eglGetProcAddress("eglCreateStreamKHR"));
    destroy_stream = reinterpret_cast<PFNEGLDESTROYSTREAMKHRPROC>(eglGetProcAddress("eglDestroyStreamKHR"));
    stream_attrib = reinterpret_cast<PFNEGLSTREAMATTRIBKHRPROC>(eglGetProcAddress("eglStreamAttribKHR"));
    query_stream = reinterpret_cast<PFNEGLQUERYSTREAMKHRPROC>(eglGetProcAddress("eglQueryStreamKHR"));
    query_stream_u64 = reinterpret_cast<PFNEGLQUERYSTREAMU64KHRPROC>(eglGetProcAddress("eglQueryStreamu64KHR"));
    create_stream_producer_surface = reinterpret_cast<PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC>(eglGetProcAddress("eglCreateStreamProducerSurfaceKHR"));
    stream_consumer_output = reinterpret_cast<PFNEGLSTREAMCONSUMEROUTPUTEXTPROC>(eglGetProcAddress("eglStreamConsumerOutputEXT"));
    get_output_layers = reinterpret_cast<PFNEGLGETOUTPUTLAYERSEXTPROC>(eglGetProcAddress("eglGetOutputLayersEXT"));
    get_output_ports = reinterpret_cast<PFNEGLGETOUTPUTPORTSEXTPROC>(eglGetProcAddress("eglGetOutputPortsEXT"));
    output_layer_attrib = reinterpret_cast<PFNEGLOUTPUTLAYERATTRIBEXTPROC>(eglGetProcAddress("eglOutputLayerAttribEXT"));
    query_output_layer_attrib = reinterpret_cast<PFNEGLQUERYOUTPUTLAYERATTRIBEXTPROC>(eglGetProcAddress("eglQueryOutputLayerAttribEXT"));
    query_output_layer_string = reinterpret_cast<PFNEGLQUERYOUTPUTLAYERSTRINGEXTPROC>(eglGetProcAddress("eglQueryOutputLayerStringEXT"));
    query_output_port_attrib = reinterpret_cast<PFNEGLQUERYOUTPUTPORTATTRIBEXTPROC>(eglGetProcAddress("eglQueryOutputPortAttribEXT"));
    query_output_port_string = reinterpret_cast<PFNEGLQUERYOUTPUTPORTSTRINGEXTPROC>(eglGetProcAddress("eglQueryOutputPortStringEXT"));
    get_stream_file_descriptor = reinterpret_cast<PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC>(eglGetProcAddress("eglGetStreamFileDescriptorKHR"));
    create_stream_from_file_descriptor = reinterpret_cast<PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC>(eglGetProcAddress("eglCreateStreamFromFileDescriptorKHR"));
    stream_consumer_gltexture = reinterpret_cast<PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC>(eglGetProcAddress("eglStreamConsumerGLTextureExternalKHR"));
    stream_consumer_acquire = reinterpret_cast<PFNEGLSTREAMCONSUMERACQUIREKHRPROC>(eglGetProcAddress("eglStreamConsumerAcquireKHR"));
    stream_consumer_release = reinterpret_cast<PFNEGLSTREAMCONSUMERRELEASEKHRPROC>(eglGetProcAddress("eglStreamConsumerReleaseKHR"));

    has_egl_stream = strstr(extensions, "EGL_KHR_stream");
    has_egl_stream_producer_eglsurface = strstr(extensions, "EGL_KHR_stream_producer_eglsurface");
    has_egl_stream_consumer_egloutput = strstr(extensions, "EGL_EXT_stream_consumer_egloutput");
    has_egl_output_drm = strstr(extensions, "EGL_EXT_output_drm");
    has_egl_output_base = strstr(extensions, "EGL_EXT_output_base");
    has_egl_stream_cross_process_fd = strstr(extensions, "EGL_KHR_stream_cross_process_fd");
    has_egl_stream_consumer_gltexture = strstr(extensions, "EGL_KHR_stream_consumer_gltexture");

    initialized = true;
}

} // namespace Platform

} // namespace GreenIsland
