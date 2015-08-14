/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
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

#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>

#include "mimehelper_p.h"

namespace GreenIsland {

QByteArray MimeHelper::getByteArray(QMimeData *mimeData, const QString &mimeType)
{
    QByteArray content;
    if (mimeType == QLatin1String("text/plain")) {
        content = mimeData->text().toUtf8();
    } else if (mimeData->hasImage()
               && (mimeType == QLatin1String("application/x-qt-image")
                   || mimeType.startsWith(QLatin1String("image/")))) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            QBuffer buf;
            buf.open(QIODevice::ReadWrite);
            QByteArray fmt = "BMP";
            if (mimeType.startsWith(QLatin1String("image/"))) {
                QByteArray imgFmt = mimeType.mid(6).toUpper().toLatin1();
                if (QImageWriter::supportedImageFormats().contains(imgFmt))
                    fmt = imgFmt;
            }
            QImageWriter wr(&buf, fmt);
            wr.write(image);
            content = buf.buffer();
        }
    } else if (mimeType == QLatin1String("application/x-color")) {
        content = qvariant_cast<QColor>(mimeData->colorData()).name().toLatin1();
    } else if (mimeType == QLatin1String("text/uri-list")) {
        QList<QUrl> urls = mimeData->urls();
        for (int i = 0; i < urls.count(); ++i) {
            content.append(urls.at(i).toEncoded());
            content.append('\n');
        }
    } else {
        content = mimeData->data(mimeType);
    }
    return content;
}

} // namespace GreenIsland
