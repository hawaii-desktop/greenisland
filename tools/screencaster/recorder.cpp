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

#include <QtCore/QSize>
#include <QtGui/QImage>

#include "recorder.h"

#define VIDEO_MAGIC 0x47434150

Recorder::Recorder(const QString &fileName)
{
    // Create the file
    m_file = new QFile(fileName);

    // Prepare the stream
    m_stream.setByteOrder(QDataStream::LittleEndian);
    m_stream.setVersion(QDataStream::Qt_5_0);
    m_stream << quint32(VIDEO_MAGIC);
}

Recorder::~Recorder()
{
    if (m_file->isOpen())
        m_file->close();
    delete m_file;
}

bool Recorder::open()
{
    bool result = m_file->open(QFile::WriteOnly | QFile::Truncate);
    if (result)
        m_stream.setDevice(m_file);
    return result;
}

void Recorder::write(quint32 time, const QSize &size,
                     qint32 stride, const QImage &image)
{
    m_stream << time << size << stride << image.bits();
}
