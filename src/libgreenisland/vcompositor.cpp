/****************************************************************************
 * This file is part of libGreenIsland.
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

#include <QGuiApplication>
#include <QOpenGLFunctions>

#include <qpa/qplatformnativeinterface.h>

#include "vcompositor.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

VCompositor::VCompositor(QWindow *window)
    : WaylandCompositor(window)
{
}

void VCompositor::showGraphicsInfo()
{
    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (nativeInterface) {
        EGLDisplay display = nativeInterface->nativeResourceForWindow("EglDisplay", window());
        if (display) {
            const char *str;

            str = eglQueryString(display, EGL_VERSION);
            printf("EGL version: %s\n", str ? str : "(null)");

            str = eglQueryString(display, EGL_VENDOR);
            printf("EGL vendor: %s\n", str ? str : "(null)");

            str = eglQueryString(display, EGL_CLIENT_APIS);
            printf("EGL client APIs: %s\n", str ? str : "(null)");

            str = eglQueryString(display, EGL_EXTENSIONS);
            printf("EGL extensions: %s\n", str ? str : "(null)");

            str = (char *)glGetString(GL_VERSION);
            printf("GL version: %s\n", str ? str : "(null)");

            str = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
            printf("GLSL version: %s\n", str ? str : "(null)");

            str = (char *)glGetString(GL_VENDOR);
            printf("GL vendor: %s\n", str ? str : "(null)");

            str = (char *)glGetString(GL_RENDERER);
            printf("GL renderer: %s\n", str ? str : "(null)");

            str = (char *)glGetString(GL_EXTENSIONS);
            printf("GL extensions: %s\n", str ? str : "(null)");
        }
    }
}

void VCompositor::runShell()
{
}
