/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef COMPOSITORLAUNCHER_H
#define COMPOSITORLAUNCHER_H

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>

#include "compositorprocess.h"

Q_DECLARE_LOGGING_CATEGORY(COMPOSITOR)

class CompositorLauncher : public QObject
{
    Q_OBJECT
public:
    enum Mode {
        UnknownMode = 0,
        EglFSMode,
        HwComposerMode,
        NestedMode,
        X11Mode,
        WaylandMode
    };

    enum Hardware {
        UnknownHardware = 0,
        DrmHardware,
        BroadcomHardware
    };

    CompositorLauncher(const QString &program,
                       const QStringList &arguments,
                       QObject *parent = 0);

    Mode mode() const;
    void setMode(const Mode &mode);

public Q_SLOTS:
    void start();
    void stop();

private:
    QString m_program;
    QStringList m_arguments;
    Mode m_mode;
    Hardware m_hardware;
    QString m_seat;
    QString m_socketName;
    bool m_hasLibInputPlugin;
    CompositorProcess *m_weston;
    CompositorProcess *m_compositor;

    void detectMode();
    void detectHardware();

    QString deviceModel() const;
    bool pathsExist(const QStringList &paths) const;

    void detectLibInput();

    QStringList compositorArgs() const;
    QProcessEnvironment compositorEnv() const;

    void printSummary();

    void setupEnvironment();

    void spawnCompositor();
};

#endif // COMPOSITORLAUNCHER_H
