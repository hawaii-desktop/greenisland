/****************************************************************************
**
** Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLAND_SURFACE_H
#define GREENISLAND_SURFACE_H

#include <greenisland/greenisland_export.h>

#include <QtCore/QScopedPointer>
#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtCore/QVariantMap>

struct wl_client;
struct wl_resource;

class QTouchEvent;

namespace GreenIsland {

class ClientConnection;
class SurfacePrivate;
class AbstractCompositor;
class BufferRef;
class SurfaceView;
class SurfaceInterface;
class SurfaceOperation;
class AbstractOutput;

class WlSurface;
class SurfacePrivate;
class WlExtendedSurface;

class GREENISLAND_EXPORT AbstractBufferAttacher
{
public:
    virtual ~AbstractBufferAttacher() {}

protected:
    virtual void attach(const BufferRef &ref) = 0;
    virtual void unmap() = 0;

    friend class GreenIsland::WlSurface;
};

class GREENISLAND_EXPORT Surface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Surface)
    Q_PROPERTY(ClientConnection *client READ client CONSTANT)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(Surface::WindowFlags windowFlags READ windowFlags NOTIFY windowFlagsChanged)
    Q_PROPERTY(Surface::WindowType windowType READ windowType NOTIFY windowTypeChanged)
    Q_PROPERTY(Qt::ScreenOrientation contentOrientation READ contentOrientation NOTIFY contentOrientationChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(Qt::ScreenOrientations orientationUpdateMask READ orientationUpdateMask NOTIFY orientationUpdateMaskChanged)
    Q_PROPERTY(QWindow::Visibility visibility READ visibility WRITE setVisibility NOTIFY visibilityChanged)
    Q_PROPERTY(Surface *transientParent READ transientParent)
    Q_PROPERTY(QPointF transientOffset READ transientOffset)

    Q_ENUMS(WindowFlag WindowType)
    Q_FLAGS(WindowFlag WindowFlags)

public:
    enum WindowFlag {
        OverridesSystemGestures     = 0x0001,
        StaysOnTop                  = 0x0002,
        BypassWindowManager         = 0x0004
    };
    Q_DECLARE_FLAGS(WindowFlags, WindowFlag)

    enum WindowType {
        None,
        Toplevel,
        Transient,
        Popup
    };

    enum Type {
        Invalid,
        Shm,
        Texture
    };

    Surface(wl_client *client, quint32 id, int version, AbstractCompositor *compositor);
    virtual ~Surface();

    ClientConnection *client() const;

    void addInterface(SurfaceInterface *interface);
    void removeInterface(SurfaceInterface *interface);

    Type type() const;
    bool isYInverted() const;

    bool visible() const;
    bool isMapped() const;

    QSize size() const;
    Q_INVOKABLE void requestSize(const QSize &size);

    Qt::ScreenOrientations orientationUpdateMask() const;
    Qt::ScreenOrientation contentOrientation() const;

    WindowFlags windowFlags() const;

    WindowType windowType() const;

    QWindow::Visibility visibility() const;
    void setVisibility(QWindow::Visibility visibility);
    Q_INVOKABLE void sendOnScreenVisibilityChange(bool visible); // Compat

    Surface *transientParent() const;

    QPointF transientOffset() const;

    GreenIsland::WlSurface *handle();

    QByteArray authenticationToken() const;
    QVariantMap windowProperties() const;
    void setWindowProperty(const QString &name, const QVariant &value);

    AbstractCompositor *compositor() const;

    AbstractOutput *mainOutput() const;
    void setMainOutput(AbstractOutput *mainOutput);

    QList<AbstractOutput *> outputs() const;

    QString className() const;

    QString title() const;

    bool hasInputPanelSurface() const;

    bool transientInactive() const;

    bool inputRegionContains(const QPoint &p) const;

    Q_INVOKABLE void destroy();
    Q_INVOKABLE void destroySurface();
    Q_INVOKABLE void ping();

    void ref();
    void setMapped(bool mapped);

    void setBufferAttacher(AbstractBufferAttacher *attacher);
    AbstractBufferAttacher *bufferAttacher() const;

    QList<SurfaceView *> views() const;
    QList<SurfaceInterface *> interfaces() const;

    bool sendInterfaceOp(SurfaceOperation &op);

    static Surface *fromResource(::wl_resource *resource);

public Q_SLOTS:
    void updateSelection();

protected:
    Surface(SurfacePrivate *dptr);

Q_SIGNALS:
    void mapped();
    void unmapped();
    void damaged(const QRegion &rect);
    void parentChanged(Surface *newParent, Surface *oldParent);
    void sizeChanged();
    void windowPropertyChanged(const QString &name, const QVariant &value);
    void windowFlagsChanged(WindowFlags flags);
    void windowTypeChanged(WindowType type);
    void contentOrientationChanged();
    void orientationUpdateMaskChanged();
    void extendedSurfaceReady();
    void classNameChanged();
    void titleChanged();
    void raiseRequested();
    void lowerRequested();
    void visibilityChanged();
    void pong();
    void surfaceDestroyed();

    void viewAdded(SurfaceView *view);
    void viewRemoved(SurfaceView *view);

    void configure(bool hasBuffer);
    void redraw();

    friend class SurfaceView;
    friend class SurfaceInterface;
    friend class GreenIsland::WlSurface;
};

class SurfaceUnmapLockPrivate;
class GREENISLAND_EXPORT SurfaceUnmapLock
{
public:
    SurfaceUnmapLock(Surface *surface);
    ~SurfaceUnmapLock();

private:
    SurfaceUnmapLockPrivate *const d;
};

typedef QList<Surface *> SurfaceList;

} // namespace GreenIsland

#endif // GREENISLAND_SURFACE_H
