/****************************************************************************
 * This file is part of Green Island.
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

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLFunctions>
#include <QtQml/QQmlContext>

#include <qpa/qplatformnativeinterface.h>

#include "greenislandcompositor.h"
#include "greenislandcompositor_p.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

Q_LOGGING_CATEGORY(GREENISLAND_COMPOSITOR, "greenisland.compositor")

using namespace GreenIsland;

/*
 * CompositorPrivate
 */

CompositorPrivate::CompositorPrivate(Compositor *parent)
    : q_ptr(parent)
    , shellProcess(nullptr)
{
}

CompositorPrivate::~CompositorPrivate()
{
    closeShell();
}

void CompositorPrivate::closeShell()
{
    if (!shellProcess)
        return;

    shellProcess->close();
    delete shellProcess;
    shellProcess = nullptr;
}

void CompositorPrivate::_q_shellStarted()
{
}

void CompositorPrivate::_q_shellFailed(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "The shell process failed to start.\n"
                << "Either the invoked program is missing, or you may have insufficient permissions to run it.";
        break;
    case QProcess::Crashed:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "The shell process crashed some time after starting successfully.";
        break;
    case QProcess::Timedout:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "The shell process timedout.\n";
        break;
    case QProcess::WriteError:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "An error occurred when attempting to write to the shell process.";
        break;
    case QProcess::ReadError:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "An error occurred when attempting to read from the shell process.";
        break;
    case QProcess::UnknownError:
        qCWarning(GREENISLAND_COMPOSITOR)
                << "Unknown error starting the shell process!";
        break;
    }

    // Don't need it anymore because it failed
    shellProcess->close();
    delete shellProcess;
    shellProcess = nullptr;
}

void CompositorPrivate::_q_shellReadyReadStandardOutput()
{
    if (shellProcess)
        printf("%s", shellProcess->readAllStandardOutput().constData());
}

void CompositorPrivate::_q_shellReadyReadStandardError()
{
    if (shellProcess)
        fprintf(stderr, "%s", shellProcess->readAllStandardError().constData());
}

void CompositorPrivate::_q_shellAboutToClose()
{
    qCDebug(GREENISLAND_COMPOSITOR) << "Shell is about to close...";
}

/*
 * Compositor
 */

Compositor::Compositor(const char *socketName, QWaylandCompositor::ExtensionFlag extensions)
    : QWaylandCompositor(this, socketName, extensions)
    , d_ptr(new CompositorPrivate(this))
{
    // Default window title
    setTitle(QLatin1String("GreenIsland"));

    // Allow QML to access this compositor
    rootContext()->setContextProperty("compositor", this);

    // Default settings
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::black);

    // Create platform window
    winId();
}

Compositor::~Compositor()
{
    delete d_ptr;
}

QString Compositor::shellFileName() const
{
    Q_D(const Compositor);
    return d->shellFileName;
}

void Compositor::setShellFileName(const QString &fileName)
{
    Q_D(Compositor);

    if (d->shellFileName != fileName) {
        d->shellFileName = fileName;
        Q_EMIT shellFileNameChanged(fileName);
    }
}

void Compositor::showGraphicsInfo()
{
    const char *str;

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (nativeInterface) {
        EGLDisplay display = nativeInterface->nativeResourceForWindow("EglDisplay", window());
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
                                            << qPrintable(extensions.join(QStringLiteral("\n\t")));
        }
    }

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
                                    << qPrintable(extensions.join(QStringLiteral("\n\t")));
}

void Compositor::runShell(const QStringList &arguments)
{
    Q_D(Compositor);

    // Sanity check
    if (d->shellFileName.isEmpty() || d->shellProcess)
        return;

    // Run the shell client process
    d->shellProcess = new QProcess(this);
    connect(d->shellProcess, SIGNAL(started()),
            this, SLOT(_q_shellStarted()));
    connect(d->shellProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(_q_shellFailed(QProcess::ProcessError)));
    connect(d->shellProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(_q_shellReadyReadStandardOutput()));
    connect(d->shellProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(_q_shellReadyReadStandardError()));
    connect(d->shellProcess, SIGNAL(aboutToClose()),
            this, SLOT(_q_shellAboutToClose()));
    d->shellProcess->start(d->shellFileName, arguments, QIODevice::ReadOnly);
}

void Compositor::closeShell()
{
    Q_D(Compositor);
    d->closeShell();
}

void Compositor::resizeEvent(QResizeEvent *event)
{
    // Scale compositor output to window's size
    QQuickView::resizeEvent(event);
    QWaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

#include "moc_greenislandcompositor.cpp"
