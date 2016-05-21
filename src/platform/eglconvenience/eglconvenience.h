/****************************************************************************
 * This file is part of Hawaii.
 *
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
