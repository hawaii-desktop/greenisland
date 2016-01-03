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

#ifndef GREENISLAND_EGLCONVENIENCE_H
#define GREENISLAND_EGLCONVENIENCE_H

#include <QtGui/QSurfaceFormat>
#include <QtCore/QVector>
#include <QtCore/QSizeF>

#include <GreenIsland/platform/greenislandplatform_export.h>

#include <EGL/egl.h>

namespace GreenIsland {

namespace Platform {

class GREENISLANDPLATFORM_EXPORT EglUtils
{
public:
    static QVector<EGLint> createConfigAttributesFromFormat(const QSurfaceFormat &format);
    static bool reduceConfigAttributes(QVector<EGLint> *configAttributes);
    static EGLConfig configFromGLFormat(EGLDisplay display,
                                        const QSurfaceFormat &format,
                                        bool highestPixelFormat = false,
                                        int surfaceType = EGL_WINDOW_BIT);
    static QSurfaceFormat glFormatFromConfig(EGLDisplay display,
                                             const EGLConfig config,
                                             const QSurfaceFormat &referenceFormat = QSurfaceFormat());
    static bool hasEglExtension(EGLDisplay display,const char* extensionName);
    static void printEglConfig(EGLDisplay display, EGLConfig config);

#ifdef Q_OS_UNIX
    static QSizeF physicalScreenSizeFromFb(int framebufferDevice,
                                           const QSize &screenSize = QSize());
    static QSize screenSizeFromFb(int framebufferDevice);
    static int screenDepthFromFb(int framebufferDevice);
    static qreal refreshRateFromFb(int framebufferDevice);
#endif
};

class GREENISLANDPLATFORM_EXPORT EglConfigChooser
{
public:
    EglConfigChooser(EGLDisplay display);
    virtual ~EglConfigChooser();

    EGLDisplay display() const { return m_display; }

    void setSurfaceType(EGLint surfaceType) { m_surfaceType = surfaceType; }
    EGLint surfaceType() const { return m_surfaceType; }

    void setSurfaceFormat(const QSurfaceFormat &format) { m_format = format; }
    QSurfaceFormat surfaceFormat() const { return m_format; }

    void setIgnoreColorChannels(bool ignore) { m_ignore = ignore; }
    bool ignoreColorChannels() const { return m_ignore; }

    EGLConfig chooseConfig();

protected:
    virtual bool filterConfig(EGLConfig config) const;

    QSurfaceFormat m_format;
    EGLDisplay m_display;
    EGLint m_surfaceType;
    bool m_ignore;

    int m_confAttrRed;
    int m_confAttrGreen;
    int m_confAttrBlue;
    int m_confAttrAlpha;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLCONVENIENCE_H
