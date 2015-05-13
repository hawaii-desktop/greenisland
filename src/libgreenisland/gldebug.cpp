/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QWindow>

#include <qpa/qplatformnativeinterface.h>

#include "gldebug.h"
#include "logging.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace GreenIsland {

static QString wordWrap(const QString &str, int wrapAt = 55)
{
    QString tmpStr = str;
    int curLen = wrapAt;

    while (curLen < str.length()) {
        int spacePos = tmpStr.indexOf(' ', curLen);
        if (spacePos == -1)
            spacePos = tmpStr.indexOf(' ', curLen);
        if (spacePos != -1) {
            tmpStr.replace(spacePos, 1, "\n\t");
            curLen = spacePos + wrapAt + 1;
        }
    }

    return tmpStr;
}

void printGraphicsInformation(QWindow *window)
{
    if (!window)
        return;

    static bool alreadyShown = false;

    if (alreadyShown)
        return;

    alreadyShown = true;

    const char *str;

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (nativeInterface) {
        EGLDisplay display = nativeInterface->nativeResourceForWindow("EglDisplay", window);
        if (display) {
            str = eglQueryString(display, EGL_VERSION);
            qCDebug(GREENISLAND_COMPOSITOR) << "EGL version:" << str;

            str = eglQueryString(display, EGL_VENDOR);
            qCDebug(GREENISLAND_COMPOSITOR) << "EGL vendor:" << str;

            str = eglQueryString(display, EGL_CLIENT_APIS);
            qCDebug(GREENISLAND_COMPOSITOR) << "EGL client APIs:" << str;

            str = eglQueryString(display, EGL_EXTENSIONS);
            QStringList extensions = QString(str).split(QLatin1Char(' '));
            qCDebug(GREENISLAND_COMPOSITOR) << "EGL extensions:"
                                            << qPrintable(wordWrap(extensions.join(" ")));
        }
    }

#ifdef QT_COMPOSITOR_WAYLAND_GL
    str = (char *)glGetString(GL_VERSION);
    qCDebug(GREENISLAND_COMPOSITOR) << "GL version:" << str;

    str = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    qCDebug(GREENISLAND_COMPOSITOR) << "GLSL version:" << str;

    str = (char *)glGetString(GL_VENDOR);
    qCDebug(GREENISLAND_COMPOSITOR) << "GL vendor:" << str;

    str = (char *)glGetString(GL_RENDERER);
    qCDebug(GREENISLAND_COMPOSITOR) << "GL renderer:" << str;

    str = (char *)glGetString(GL_EXTENSIONS);
    QStringList extensions = QString(str).split(QLatin1Char(' '));
    qCDebug(GREENISLAND_COMPOSITOR) << "GL extensions:"
                                    << qPrintable(wordWrap(extensions.join(" ")));
#endif
}

}
