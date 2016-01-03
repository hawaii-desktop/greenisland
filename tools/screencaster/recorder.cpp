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
