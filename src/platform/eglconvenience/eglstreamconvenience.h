/****************************************************************************
 * This file is part of Green Island.
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

#ifndef GREENISLAND_EGLSTREAMCONVENIENCE_H
#define GREENISLAND_EGLSTREAMCONVENIENCE_H

#include <qglobal.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

// This provides runtime EGLDevice/Output/Stream support even when eglext.h in
// the sysroot is not up-to-date.

#ifndef EGL_VERSION_1_5
typedef intptr_t EGLAttrib;
#endif

#ifndef EGL_EXT_platform_base
typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);
#endif

#ifndef EGL_EXT_device_base
typedef void *EGLDeviceEXT;
#define EGL_NO_DEVICE_EXT                 ((EGLDeviceEXT)(0))
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYDEVICESEXTPROC) (EGLint max_devices, EGLDeviceEXT *devices, EGLint *num_devices);
typedef const char *(EGLAPIENTRYP PFNEGLQUERYDEVICESTRINGEXTPROC) (EGLDeviceEXT device, EGLint name);
#endif

#ifndef EGL_EXT_output_base
typedef void *EGLOutputLayerEXT;
typedef void *EGLOutputPortEXT;
#define EGL_NO_OUTPUT_LAYER_EXT           ((EGLOutputLayerEXT)0)
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETOUTPUTLAYERSEXTPROC) (EGLDisplay dpy, const EGLAttrib *attrib_list, EGLOutputLayerEXT *layers, EGLint max_layers, EGLint *num_layers);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETOUTPUTPORTSEXTPROC) (EGLDisplay dpy, const EGLAttrib *attrib_list, EGLOutputPortEXT *ports, EGLint max_ports, EGLint *num_ports);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLOUTPUTLAYERATTRIBEXTPROC) (EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint attribute, EGLAttrib value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYOUTPUTLAYERATTRIBEXTPROC) (EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint attribute, EGLAttrib *value);
typedef const char *(EGLAPIENTRYP PFNEGLQUERYOUTPUTLAYERSTRINGEXTPROC) (EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint name);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYOUTPUTPORTATTRIBEXTPROC) (EGLDisplay dpy, EGLOutputPortEXT port, EGLint attribute, EGLAttrib *value);
typedef const char *(EGLAPIENTRYP PFNEGLQUERYOUTPUTPORTSTRINGEXTPROC) (EGLDisplay dpy, EGLOutputPortEXT port, EGLint name);
#endif

#ifndef EGL_KHR_stream
typedef void *EGLStreamKHR;
typedef quint64 EGLuint64KHR;
#define EGL_NO_STREAM_KHR                 ((EGLStreamKHR)0)
typedef EGLStreamKHR (EGLAPIENTRYP PFNEGLCREATESTREAMKHRPROC) (EGLDisplay dpy, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYSTREAMKHRPROC) (EGLDisplay dpy, EGLStreamKHR stream);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSTREAMATTRIBKHRPROC) (EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLint value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSTREAMKHRPROC) (EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLint *value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSTREAMU64KHRPROC) (EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLuint64KHR *value);
#endif

#ifndef EGL_KHR_stream_producer_eglsurface
#define EGL_STREAM_BIT_KHR                0x0800
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC) (EGLDisplay dpy, EGLConfig config, EGLStreamKHR stream, const EGLint *attrib_list);
#endif

#ifndef EGL_KHR_stream_cross_process_fd
typedef int EGLNativeFileDescriptorKHR;
#define EGL_NO_FILE_DESCRIPTOR_KHR        ((EGLNativeFileDescriptorKHR)(-1))
typedef EGLNativeFileDescriptorKHR (EGLAPIENTRYP PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC) (EGLDisplay dpy, EGLStreamKHR stream);
typedef EGLStreamKHR (EGLAPIENTRYP PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC) (EGLDisplay dpy, EGLNativeFileDescriptorKHR file_descriptor);
#endif

#ifndef EGL_KHR_stream_consumer_gltexture
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC) (EGLDisplay dpy, EGLStreamKHR stream);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSTREAMCONSUMERACQUIREKHRPROC) (EGLDisplay dpy, EGLStreamKHR stream);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSTREAMCONSUMERRELEASEKHRPROC) (EGLDisplay dpy, EGLStreamKHR stream);
#endif

#ifndef EGL_EXT_stream_consumer_egloutput
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSTREAMCONSUMEROUTPUTEXTPROC) (EGLDisplay dpy, EGLStreamKHR stream, EGLOutputLayerEXT layer);
#endif

#ifndef EGL_EXT_platform_device
#define EGL_PLATFORM_DEVICE_EXT           0x313F
#endif

#ifndef EGL_EXT_device_drm
#define EGL_DRM_DEVICE_FILE_EXT           0x3233
#endif

#ifndef EGL_EXT_output_drm
#define EGL_DRM_CRTC_EXT                  0x3234
#define EGL_DRM_PLANE_EXT                 0x3235
#endif

namespace GreenIsland {

namespace Platform {

class GREENISLANDPLATFORM_EXPORT EGLStreamConvenience
{
public:
    EGLStreamConvenience();
    void initialize(EGLDisplay dpy);

    PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display;
    PFNEGLQUERYDEVICESEXTPROC query_devices;
    PFNEGLQUERYDEVICESTRINGEXTPROC query_device_string;
    PFNEGLCREATESTREAMKHRPROC create_stream;
    PFNEGLDESTROYSTREAMKHRPROC destroy_stream;
    PFNEGLSTREAMATTRIBKHRPROC stream_attrib;
    PFNEGLQUERYSTREAMKHRPROC query_stream;
    PFNEGLQUERYSTREAMU64KHRPROC query_stream_u64;
    PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC create_stream_producer_surface;
    PFNEGLSTREAMCONSUMEROUTPUTEXTPROC stream_consumer_output;
    PFNEGLGETOUTPUTLAYERSEXTPROC get_output_layers;
    PFNEGLGETOUTPUTPORTSEXTPROC get_output_ports;
    PFNEGLOUTPUTLAYERATTRIBEXTPROC output_layer_attrib;
    PFNEGLQUERYOUTPUTLAYERATTRIBEXTPROC query_output_layer_attrib;
    PFNEGLQUERYOUTPUTLAYERSTRINGEXTPROC query_output_layer_string;
    PFNEGLQUERYOUTPUTPORTATTRIBEXTPROC query_output_port_attrib;
    PFNEGLQUERYOUTPUTPORTSTRINGEXTPROC query_output_port_string;
    PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC get_stream_file_descriptor;
    PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC create_stream_from_file_descriptor;
    PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC stream_consumer_gltexture;
    PFNEGLSTREAMCONSUMERACQUIREKHRPROC stream_consumer_acquire;
    PFNEGLSTREAMCONSUMERRELEASEKHRPROC stream_consumer_release;

    bool initialized;

    bool has_egl_platform_device;
    bool has_egl_device_base;
    bool has_egl_stream;
    bool has_egl_stream_producer_eglsurface;
    bool has_egl_stream_consumer_egloutput;
    bool has_egl_output_drm;
    bool has_egl_output_base;
    bool has_egl_stream_cross_process_fd;
    bool has_egl_stream_consumer_gltexture;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLSTREAMCONVENIENCE_H