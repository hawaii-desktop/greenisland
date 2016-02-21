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

#ifndef GREENISLANDCLIENT_BUFFER_H
#define GREENISLANDCLIENT_BUFFER_H

#include <QtCore/QObject>

#include <GreenIsland/Client/Shm>

namespace GreenIsland {

namespace Client {

class BufferPrivate;
class ShmPool;
class ShmPoolPrivate;

class GREENISLANDCLIENT_EXPORT Buffer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Buffer)
    Q_PROPERTY(uchar *address READ address CONSTANT)
    Q_PROPERTY(Shm::Format format READ format CONSTANT)
    Q_PROPERTY(QSize size READ size CONSTANT)
    Q_PROPERTY(quint32 stride READ stride CONSTANT)
    Q_PROPERTY(QImage image READ image CONSTANT)
    Q_PROPERTY(bool released READ isReleased WRITE setReleased NOTIFY releasedChanged)
    Q_PROPERTY(bool used READ isUsed WRITE setUsed NOTIFY usedChanged)
public:
    uchar *address() const;
    Shm::Format format() const;
    QSize size() const;
    qint32 stride() const;
    QImage image() const;

    bool isReleased() const;
    void setReleased(bool released);

    bool isUsed() const;
    void setUsed(bool used);

    void copy(const void *source);

Q_SIGNALS:
    void releasedChanged();
    void usedChanged();

private:
    explicit Buffer(ShmPool *pool, const QSize &size, qint32 stride,
                    qint32 offset, Shm::Format format);

    friend class ShmPoolPrivate;
};

typedef QWeakPointer<Buffer> BufferPtr;
typedef QSharedPointer<Buffer> BufferSharedPtr;

} // namespace Client

} //namespace GreenIsland

#endif // GREENISLANDCLIENT_BUFFER_H
