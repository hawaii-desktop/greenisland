/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>

#include "cmakedirs.h"
#include "compositorlauncher.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(COMPOSITOR, "greenisland.launcher")

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#define qCInfo qCDebug
#endif

CompositorLauncher::CompositorLauncher(const QString &program, const QStringList &arguments,
                                       QObject *parent)
    : QObject(parent)
    , m_program(program)
    , m_arguments(arguments)
    , m_mode(UnknownMode)
    , m_hardware(UnknownHardware)
    , m_hasLibInputPlugin(false)
    , m_weston(Q_NULLPTR)
    , m_compositor(Q_NULLPTR)
{
    // Detect seat or fallback to seat0
    m_seat = qEnvironmentVariableIsSet("XDG_SEAT")
            ? qgetenv("XDG_SEAT")
            : QStringLiteral("seat0");
}

CompositorLauncher::Mode CompositorLauncher::mode() const
{
    return m_mode;
}

void CompositorLauncher::setMode(const Mode &mode)
{
    m_mode = mode;
}

void CompositorLauncher::start()
{
    // Try to detect mode and hardware
    detectHardware();
    detectMode();
    if (m_mode == UnknownMode) {
        qCWarning(COMPOSITOR) << "No mode detected, please manually specify one!";
        QCoreApplication::quit();
        return;
    }

    // Detect whether we have libinput
    detectLibInput();

    // Setup the sockets
    if (m_mode == NestedMode) {
        // Weston
        m_weston = new CompositorProcess(this);
        m_weston->setSocketName(QStringLiteral("greenisland-master-") + m_seat);
        m_weston->setProgram(QStringLiteral("weston"));
        m_weston->setArguments(QStringList()
                               << QStringLiteral("--shell=fullscreen-shell.so")
                               << QStringLiteral("--socket=%1").arg(m_weston->socketName()));

        // Compositor
        m_socketName = QStringLiteral("greenisland-slave-") + m_seat;
        m_compositor = new CompositorProcess(this);
        m_compositor->setSocketName(m_socketName);
        m_compositor->setProgram(m_program);
        m_compositor->setArguments(compositorArgs());
        m_compositor->setEnvironment(compositorEnv());

        // Starts the compositor as soon as Weston is started
        connect(m_weston, &CompositorProcess::started,
                m_compositor, &CompositorProcess::start);
    } else {
        // Compositor socket name
        m_socketName = QStringLiteral("greenisland-") + m_seat;
    }

    // Summary
    printSummary();

    // Start the process
    if (m_mode == NestedMode)
        m_weston->start();
    else
        spawnCompositor();
}

void CompositorLauncher::stop()
{
    if (m_mode != NestedMode)
        return;

    m_compositor->stop();
    m_weston->stop();
}

void CompositorLauncher::detectMode()
{
    // Can't detect mode when it was forced by arguments
    if (m_mode != UnknownMode)
        return;

    // Detect Wayland
    if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
        m_mode = WaylandMode;
        return;
    }

    // Detect X11
    if (qEnvironmentVariableIsSet("DISPLAY")) {
        m_mode = X11Mode;
        return;
    }

    // Use eglfs mode if we detected a particular hardware except
    // for drm with Qt < 5.5 because eglfs_kms was not available
    if (m_hardware != UnknownHardware) {
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
        if (m_hardware == DrmHardware) {
            m_mode = NestedMode;
            return;
        }
#endif
        m_mode = EglFSMode;
        return;
    }

    // TODO: Detect hwcomposer

    // Default to nested mode
    m_mode = NestedMode;
}

void CompositorLauncher::detectHardware()
{
    // Detect Broadcom
    bool found = deviceModel().startsWith(QStringLiteral("Raspberry"));
    if (!found) {
        QStringList paths = QStringList()
                << QStringLiteral("/usr/bin/vcgencmd")
                << QStringLiteral("/opt/vc/bin/vcgencmd")
                << QStringLiteral("/proc/vc-cma");
        found = pathsExist(paths);
    }
    if (found) {
        m_hardware = BroadcomHardware;
        return;
    }

    // TODO: Detect Mali
    // TODO: Detect Vivante

    // Detect DRM
    if (QDir(QStringLiteral("/sys/class/drm")).exists()) {
        m_hardware = DrmHardware;
        return;
    }

    // Unknown hardware
    m_hardware = UnknownHardware;
}

QString CompositorLauncher::deviceModel() const
{
    QFile file(QStringLiteral("/proc/device-tree/model"));
    if (file.open(QIODevice::ReadOnly)) {
        QString data = QString::fromUtf8(file.readAll());
        file.close();
        return data;
    }

    return QString();
}

bool CompositorLauncher::pathsExist(const QStringList &paths) const
{
    Q_FOREACH (const QString &path, paths) {
        if (QFile::exists(path))
            return true;
    }

    return false;
}

void CompositorLauncher::detectLibInput()
{
    // Do we have the libinput plugin?
    Q_FOREACH (const QString &path, QCoreApplication::libraryPaths()) {
        QDir pluginsDir(path + QStringLiteral("/generic"));
        Q_FOREACH (const QString &fileName, pluginsDir.entryList(QDir::Files)) {
            if (fileName == QStringLiteral("libqlibinputplugin.so")) {
                m_hasLibInputPlugin = true;
                break;
            }
        }
    }
}

QStringList CompositorLauncher::compositorArgs() const
{
    QStringList args = m_arguments;

    // Specific arguments
    switch (m_mode) {
    case HwComposerMode:
        if (m_hasLibInputPlugin)
            args << QStringLiteral("-plugin") << QStringLiteral("libinput");
        break;
    case NestedMode:
        args << QStringLiteral("--nested");
        break;
    default:
        break;
    }

    // Always set a socket name so it has a known value and
    // application will have the correct WAYLAND_DISPLAY set
    args << QStringLiteral("--wayland-socket-name")
         << m_socketName;

    return args;
}

QProcessEnvironment CompositorLauncher::compositorEnv() const
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Specific environment
    switch (m_mode) {
    case NestedMode:
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("greenisland"));
        env.insert(QStringLiteral("WAYLAND_DISPLAY"), m_weston->socketName());

        if (m_hardware == BroadcomHardware) {
            env.insert(QStringLiteral("QT_WAYLAND_HARDWARE_INTEGRATION"), QStringLiteral("brcm-egl"));
            env.insert(QStringLiteral("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION"), QStringLiteral("brcm-egl"));
        }
        break;
    case EglFSMode:
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("greenisland"));
        break;
    case HwComposerMode:
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("hwcomposer"));
        env.insert(QStringLiteral("EGL_PLATFORM"), QStringLiteral("hwcomposer"));
        //env.insert(QStringLiteral("QT_COMPOSITOR_NEGATE_INVERTED_Y"), QStringLiteral("0"));
        //env.insert(QStringLiteral("QT_QPA_EGLFS_DEPTH"), QStringLiteral("32"));
        if (m_hasLibInputPlugin)
            env.insert(QStringLiteral("QT_QPA_EGLFS_DISABLE_INPUT"), QStringLiteral("1"));
        break;
    case X11Mode:
        env.insert(QStringLiteral("QT_XCB_GL_INTEGRATION"), QStringLiteral("xcb_egl"));
        break;
    case WaylandMode:
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("wayland"));
        break;
    default:
        break;
    }

    return env;
}

void CompositorLauncher::printSummary()
{
    auto modeToString = [this] {
        switch (m_mode) {
        case EglFSMode:
            return QStringLiteral("eglfs");
        case HwComposerMode:
            return QStringLiteral("hwcomposer");
        case NestedMode:
            return QStringLiteral("nested");
        case X11Mode:
            return QStringLiteral("x11");
        case WaylandMode:
            return QStringLiteral("wayland");
        default:
            return QStringLiteral("unknown");
        }
    };

    auto hwToString = [this] {
        switch (m_hardware) {
        case DrmHardware:
            return QStringLiteral("drm");
        case BroadcomHardware:
            return QStringLiteral("broadcom");
        default:
            return QStringLiteral("unknown");
        }
    };

    qCInfo(COMPOSITOR) << "Mode:" << modeToString();
    qCInfo(COMPOSITOR) << "Hardware:" << hwToString();
    qCInfo(COMPOSITOR) << "libinput:" << m_hasLibInputPlugin;
    if (m_mode == NestedMode) {
        qCInfo(COMPOSITOR) << "Weston socket:" << m_weston->socketName();
        qCInfo(COMPOSITOR) << "Compositor socket:" << m_compositor->socketName();
    }
    if (m_mode == X11Mode)
        qCInfo(COMPOSITOR) << "X11 display:" << qgetenv("DISPLAY");
}

void CompositorLauncher::setupEnvironment()
{
    return;
    // Make clients run on Wayland
    if (m_hardware == BroadcomHardware) {
        qputenv("QT_QPA_PLATFORM", QByteArray("wayland-brcm"));
        qputenv("QT_WAYLAND_HARDWARE_INTEGRATION", QByteArray("brcm-egl"));
        qputenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION", QByteArray("brcm-egl"));
    } else {
        qputenv("QT_QPA_PLATFORM", QByteArray("wayland"));
    }
    qputenv("GDK_BACKEND", QByteArray("wayland"));
}

void CompositorLauncher::spawnCompositor()
{
    // Setup the arguments
    QStringList args = compositorArgs();
    args.prepend(m_program);
    char **const argv = new char *[args.count() + 1];
    char **p = argv;

    Q_FOREACH (const QString &arg, args) {
        *p = new char[arg.length() + 1];
        ::strcpy(*p, qPrintable(arg));
        ++p;
    }

    *p = 0;

    // Setup the environment
    QProcessEnvironment env = compositorEnv();
    char **const envp = new char *[env.keys().count() + 1];
    char **e = envp;

    Q_FOREACH (const QString &key, env.keys()) {
        QString item = QString("%1=%2").arg(key).arg(env.value(key));
        *e = new char[item.length() + 1];
        ::strcpy(*e, qPrintable(item));
        ++e;
    }

    *e = 0;

    // Execute
    qCInfo(COMPOSITOR, "Running: %s", qPrintable(args.join(' ')));
    if (::execve(argv[0], argv, envp) == -1) {
        qCCritical(COMPOSITOR, "Failed to execute %s: %s",
                   argv[0], ::strerror(errno));

        p = argv;
        while (*p != 0)
            delete [] *p++;
        delete [] argv;

        e = envp;
        while (*e != 0)
            delete [] *e++;
        delete [] envp;

        ::exit(EXIT_FAILURE);
    } else {
        ::exit(0);
    }
}

#include "moc_compositorlauncher.cpp"
