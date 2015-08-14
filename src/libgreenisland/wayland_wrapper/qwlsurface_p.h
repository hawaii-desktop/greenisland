/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WL_SURFACE_H
#define WL_SURFACE_H

#include <greenisland/greenisland_export.h>

#include "surfacebuffer.h"
#include "wayland_wrapper/qwloutput_p.h"
#include "surface.h"
#include "bufferref.h"

#include <QtCore/QVector>
#include <QtCore/QRect>
#include <QtGui/QRegion>
#include <QtGui/QImage>
#include <QtGui/QWindow>

#include <QtCore/QTextStream>
#include <QtCore/QMetaType>

#include <wayland-util.h>

#include "qwayland-server-wayland.h"

class QTouchEvent;

namespace GreenIsland {

class WlCompositor;
class Buffer;

class WlExtendedSurface;
class WlInputPanelSurface;
class SubSurface;
class FrameCallback;

class SurfaceUnmapLock;
class SurfaceRole;
class RoleBase;

class GREENISLAND_EXPORT WlSurface : public QtWaylandServer::wl_surface
{
public:
    WlSurface(struct wl_client *client, uint32_t id, int version, AbstractCompositor *compositor, Surface *surface);
    ~WlSurface();

    bool setRole(const SurfaceRole *role, wl_resource *errorResource, uint32_t errorCode);
    const SurfaceRole *role() const { return m_role; }
    template<class T>
    bool setRoleHandler(T *handler);

    static WlSurface *fromResource(struct ::wl_resource *resource);

    Surface::Type type() const;
    bool isYInverted() const;

    bool mapped() const;

    using QtWaylandServer::wl_surface::resource;

    QSize size() const;
    void setSize(const QSize &size);

    QRegion inputRegion() const;
    QRegion opaqueRegion() const;

    void sendFrameCallback();
    void removeFrameCallback(FrameCallback *callback);

    Surface *waylandSurface() const;

    QPoint lastMousePos() const;

    void setExtendedSurface(WlExtendedSurface *extendedSurface);
    WlExtendedSurface *extendedSurface() const;

    void setSubSurface(SubSurface *subSurface);
    SubSurface *subSurface() const;
    void addSubSurface(SubSurface *ss);
    void removeSubSurface(SubSurface *ss);

    void setInputPanelSurface(WlInputPanelSurface *inputPanelSurface);
    WlInputPanelSurface *inputPanelSurface() const;

    WlCompositor *compositor() const;

    WlOutput *mainOutput() const;
    void setMainOutput(WlOutput *output);

    QList<WlOutput *> outputs() const;

    void addToOutput(WlOutput *output);
    void removeFromOutput(WlOutput *output);

    QString className() const { return m_className; }
    void setClassName(const QString &className);

    QString title() const { return m_title; }
    void setTitle(const QString &title);

    WlSurface *transientParent() const { return m_transientParent; }
    void setTransientParent(WlSurface *parent) { m_transientParent = parent; }

    bool transientInactive() const { return m_transientInactive; }
    void setTransientInactive(bool v) { m_transientInactive = v; }

    void setTransientOffset(qreal x, qreal y);

    bool isCursorSurface() const { return m_isCursorSurface; }
    void setCursorSurface(bool isCursor) { m_isCursorSurface = isCursor; }

    void releaseSurfaces();
    void frameStarted();

    void addUnmapLock(SurfaceUnmapLock *l);
    void removeUnmapLock(SurfaceUnmapLock *l);

    void setMapped(bool mapped);
    void setVisibility(QWindow::Visibility visibility) { m_visibility = visibility; }

    inline bool isDestroyed() const { return m_destroyed; }

    Qt::ScreenOrientation contentOrientation() const;

protected:
    void surface_destroy_resource(Resource *resource) Q_DECL_OVERRIDE;

    void surface_destroy(Resource *resource) Q_DECL_OVERRIDE;
    void surface_attach(Resource *resource,
                        struct wl_resource *buffer, int x, int y) Q_DECL_OVERRIDE;
    void surface_damage(Resource *resource,
                        int32_t x, int32_t y, int32_t width, int32_t height) Q_DECL_OVERRIDE;
    void surface_frame(Resource *resource,
                       uint32_t callback) Q_DECL_OVERRIDE;
    void surface_set_opaque_region(Resource *resource,
                                   struct wl_resource *region) Q_DECL_OVERRIDE;
    void surface_set_input_region(Resource *resource,
                                  struct wl_resource *region) Q_DECL_OVERRIDE;
    void surface_commit(Resource *resource) Q_DECL_OVERRIDE;
    void surface_set_buffer_transform(Resource *resource, int32_t transform) Q_DECL_OVERRIDE;

    Q_DISABLE_COPY(WlSurface)

    WlCompositor *m_compositor;
    Surface *m_waylandSurface;
    WlOutput *m_mainOutput;
    QList<WlOutput *> m_outputs;

    QRegion m_damage;
    SurfaceBuffer *m_buffer;
    BufferRef m_bufferRef;
    bool m_surfaceMapped;
    AbstractBufferAttacher *m_attacher;
    QVector<SurfaceUnmapLock *> m_unmapLocks;

    struct {
        SurfaceBuffer *buffer;
        QRegion damage;
        QPoint offset;
        bool newlyAttached;
        QRegion inputRegion;
    } m_pending;

    QPoint m_lastLocalMousePos;
    QPoint m_lastGlobalMousePos;

    QList<FrameCallback *> m_pendingFrameCallbacks;
    QList<FrameCallback *> m_frameCallbacks;

    WlExtendedSurface *m_extendedSurface;
    SubSurface *m_subSurface;
    WlInputPanelSurface *m_inputPanelSurface;

    QRegion m_inputRegion;
    QRegion m_opaqueRegion;

    QVector<SurfaceBuffer *> m_bufferPool;

    QSize m_size;
    QString m_className;
    QString m_title;
    WlSurface *m_transientParent;
    bool m_transientInactive;
    QPointF m_transientOffset;
    bool m_isCursorSurface;
    bool m_destroyed;
    Qt::ScreenOrientation m_contentOrientation;
    QWindow::Visibility m_visibility;
    QVector<SubSurface *> m_subsurfaces;

    const SurfaceRole *m_role;
    RoleBase *m_roleHandler;

    void setBackBuffer(SurfaceBuffer *buffer);
    SurfaceBuffer *createSurfaceBuffer(struct ::wl_resource *buffer);

    friend class Surface;
    friend class RoleBase;
};

class SurfaceRole
{
public:
    const char *name;
};

class RoleBase
{
public:
    virtual ~RoleBase() {
        if (m_surface) {
            m_surface->m_roleHandler = 0; m_surface = 0;
        }
    }

protected:
    RoleBase() : m_surface(0) {}
    static inline RoleBase *roleOf(WlSurface *s) { return s->m_roleHandler; }

    virtual void configure(int dx, int dy) = 0;

private:
    WlSurface *m_surface;
    friend class WlSurface;
};

template<class T>
class SurfaceRoleHandler : public RoleBase
{
public:
    static T *get(WlSurface *surface) {
        if (surface->role() == T::role()) {
            return static_cast<T *>(roleOf(surface));
        }
        return 0;
    }
};

template<class T>
bool WlSurface::setRoleHandler(T *handler)
{
    RoleBase *base = handler;
    if (m_role == T::role()) {
        m_roleHandler = base;
        base->m_surface = this;
        return true;
    }
    return false;
}

}


#endif //WL_SURFACE_H
