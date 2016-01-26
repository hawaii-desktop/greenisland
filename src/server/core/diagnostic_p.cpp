/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2012-2016 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QSet>
#include <QtCore/QSysInfo>
#include <QtCore/QTextStream>
#include <QtCore/private/qsimd_p.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "diagnostic_p.h"
#include "greenisland_version.h"

#include <wayland-version.h>

#define DUMP_CPU_FEATURE(feature, name)  \
    if (qCpuHasFeature(feature)) str << " " name;

extern char **environ;

namespace GreenIsland {

namespace DiagnosticOutput {

QString systemInformation()
{
    QString result;
    QTextStream str(&result);

    str << "OS: " << QSysInfo::prettyProductName();
    str << " ["
        << QSysInfo::kernelType()
        << " version " << QSysInfo::kernelVersion()
        << "]\n";
    str << "Architecture: " << QSysInfo::currentCpuArchitecture() << "; ";
    str << "features:";
#if defined(Q_PROCESSOR_X86)
    DUMP_CPU_FEATURE(SSE2, "SSE2");
    DUMP_CPU_FEATURE(SSE3, "SSE3");
    DUMP_CPU_FEATURE(SSSE3, "SSSE3");
    DUMP_CPU_FEATURE(SSE4_1, "SSE4.1");
    DUMP_CPU_FEATURE(SSE4_2, "SSE4.2");
    DUMP_CPU_FEATURE(AVX, "AVX");
    DUMP_CPU_FEATURE(AVX2, "AVX2");
    DUMP_CPU_FEATURE(RTM, "RTM");
    DUMP_CPU_FEATURE(HLE, "HLE");
#elif defined(Q_PROCESSOR_ARM)
    DUMP_CPU_FEATURE(ARM_NEON, "Neon");
#elif defined(Q_PROCESSOR_MIPS)
    DUMP_CPU_FEATURE(DSP, "DSP");
    DUMP_CPU_FEATURE(DSPR2, "DSPR2");
#endif
    str << '\n';
    return result;
}

#ifndef QT_NO_OPENGL
QTextStream &operator<<(QTextStream &str, const QSurfaceFormat &format)
{
    str << "Version: " << format.majorVersion() << '.'
        << format.minorVersion() << " Profile: " << format.profile()
        << " Swap behavior: " << format.swapBehavior()
        << " Buffer size (RGB";
    if (format.hasAlpha())
        str << 'A';
    str << "): " << format.redBufferSize() << ',' << format.greenBufferSize()
        << ',' << format.blueBufferSize();
    if (format.hasAlpha())
        str << ',' << format.alphaBufferSize();
    if (const int dbs = format.depthBufferSize())
        str << " Depth buffer: " << dbs;
    if (const int sbs = format.stencilBufferSize())
        str << " Stencil buffer: " << sbs;
    const int samples = format.samples();
    if (samples > 0)
        str << " Samples: " << samples;
    return str;
}
#endif

QString openGlContext()
{
    QString result;
    QTextStream str(&result);

    QOpenGLContext context;
    if (context.create()) {
#  ifdef QT_OPENGL_DYNAMIC
        str << "Dynamic GL ";
#  endif
        switch (context.openGLModuleType()) {
        case QOpenGLContext::LibGL:
            str << "LibGL";
            break;
        case QOpenGLContext::LibGLES:
            str << "LibGLES";
            break;
        }

        QWindow window;
        if (QGuiApplication::platformName() == QLatin1String("greenisland"))
            window.setFlags(Qt::Desktop);
        window.setSurfaceType(QSurface::OpenGLSurface);
        //window.setScreen(QGuiApplication::primaryScreen());
        window.create();
        if (context.makeCurrent(&window)) {
            QOpenGLFunctions functions(&context);

            str << " Vendor: " << reinterpret_cast<const char *>(functions.glGetString(GL_VENDOR))
                << "\nRenderer: " << reinterpret_cast<const char *>(functions.glGetString(GL_RENDERER))
                << "\nVersion: " << reinterpret_cast<const char *>(functions.glGetString(GL_VERSION))
                << "\nGLSL version: " << reinterpret_cast<const char *>(functions.glGetString(GL_SHADING_LANGUAGE_VERSION))
                << "\nFormat: " << context.format();

            QList<QByteArray> extensionList = context.extensions().toList();
            std::sort(extensionList.begin(), extensionList.end());
            str << " \nFound " << extensionList.size() << " extensions:\n";
            Q_FOREACH (const QByteArray &e, extensionList)
                str << "  " << e << '\n';

            context.doneCurrent();
        }
        window.destroy();
    } else {
        str << "Unable to create an Open GL context.\n";
    }

    return result;
}

QString framework()
{
    QString result;
    QTextStream str(&result);

    str << "Wayland version: " << WAYLAND_VERSION
        << "\nQt version: " << QT_VERSION_STR
        << "\nGreen Island version: " << GREENISLAND_VERSION_STRING
        << '\n';

    return result;
}

QString environment()
{
    QString result;
    QTextStream str(&result);

    // Environment variables
    str << "Environment variables:\n";
    for (char **current = environ; *current; current++) {
        if (::strncmp(*current, "QT", 2) == 0 ||
                ::strncmp(*current, "QML", 3) == 0 ||
                ::strncmp(*current, "QSG", 3) == 0 ||
                ::strncmp(*current, "XDG", 3) == 0)
            str << "  " << *current << '\n';
    }

    return result;
}

} // namespace DiagnosticOutput

} // namespace GreenIsland
