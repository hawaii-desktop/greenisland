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

#ifndef GREENISLANDCLIENT_SURFACE_H
#define GREENISLANDCLIENT_SURFACE_H

#include <QtCore/QObject>
#include <QtCore/QRect>

#include <GreenIsland/Client/Buffer>
#include <GreenIsland/Client/Output>

class QWindow;

namespace GreenIsland {

namespace Client {

class Output;
class Region;
class SurfacePrivate;

class GREENISLANDCLIENT_EXPORT Surface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Surface)
public:
    enum CommitMode {
        NoCommitMode,
        FrameCallbackCommitMode
    };

    Surface(QObject *parent = Q_NULLPTR);

    void setupFrameCallback();

    void attach(Buffer *buffer, const QPoint &position);
    void attach(BufferPtr buffer, const QPoint &position);

    void damage(const QRect &rect);
    void commit(CommitMode mode = FrameCallbackCommitMode);

    void setOpaqueRegion(Region *region);
    void setInputRegion(Region *region);

    void setBufferTransform(Output::Transform transform);
    void setBufferScale(qint32 scale);

    static Surface *fromQt(QWindow *window, QObject *parent = Q_NULLPTR);

    static QByteArray interfaceName();

Q_SIGNALS:
    void enter(Output *output);
    void leave(Output *output);
    void frameRendered();
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_SURFACE_H
