/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef OPENGLCOMPOSITORBACKINGSTORE_H
#define OPENGLCOMPOSITORBACKINGSTORE_H

#include <QtGui/QImage>
#include <QtGui/QRegion>
#include <QtGui/qpa/qplatformbackingstore.h>

#include <GreenIsland/platform/greenislandplatform_export.h>

class QOpenGLContext;
class QPlatformTextureList;

namespace GreenIsland {

namespace Platform {

class GREENISLANDPLATFORM_EXPORT OpenGLCompositorBackingStore : public QPlatformBackingStore
{
public:
    OpenGLCompositorBackingStore(QWindow *window);
    ~OpenGLCompositorBackingStore();

    QPaintDevice *paintDevice() Q_DECL_OVERRIDE;

    void beginPaint(const QRegion &region) Q_DECL_OVERRIDE;

    void flush(QWindow *window, const QRegion &region, const QPoint &offset) Q_DECL_OVERRIDE;
    void resize(const QSize &size, const QRegion &staticContents) Q_DECL_OVERRIDE;

    QImage toImage() const Q_DECL_OVERRIDE;
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures, QOpenGLContext *context,
                         bool translucentBackground) Q_DECL_OVERRIDE;

    const QPlatformTextureList *textures() const { return m_textures; }

    void notifyComposited();

private:
    void updateTexture();

    QWindow *m_window;
    QImage m_image;
    QRegion m_dirty;
    uint m_bsTexture;
    QPlatformTextureList *m_textures;
    QPlatformTextureList *m_lockedWidgetTextures;
};

} // namespace Platform

} // namespace GreenIsland

#endif // OPENGLCOMPOSITORBACKINGSTORE_H
