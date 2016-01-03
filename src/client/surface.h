/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
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
