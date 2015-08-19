/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
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

#ifndef GREENISLAND_OUTPUT_H
#define GREENISLAND_OUTPUT_H

#include <QtCore/QObject>
#include <QtCore/QSize>

#include <greenisland/greenisland_export.h>

struct wl_resource;

namespace GreenIsland {

class Compositor;
class FakeScreenBackend;
class QuickSurfacePrivate;
class OutputPrivate;
class OutputWindow;
class NativeScreenBackend;
class Surface;

class WlSurface;

class GREENISLAND_EXPORT Output : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Output)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString manufacturer READ manufacturer CONSTANT)
    Q_PROPERTY(QString model READ model CONSTANT)
    Q_PROPERTY(int number READ number CONSTANT)
    Q_PROPERTY(bool primary READ isPrimary NOTIFY primaryChanged)
    Q_PROPERTY(QPoint position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(int refreshRate READ refreshRate NOTIFY refreshRateChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)
    Q_PROPERTY(QRect availableGeometry READ availableGeometry WRITE setAvailableGeometry NOTIFY availableGeometryChanged)
    Q_PROPERTY(QSize physicalSize READ physicalSize WRITE setPhysicalSize NOTIFY physicalSizeChanged)
    Q_PROPERTY(Output::Subpixel subpixel READ subpixel WRITE setSubpixel NOTIFY subpixelChanged)
    Q_PROPERTY(Output::Transform transform READ transform WRITE setTransform NOTIFY transformChanged)
    Q_PROPERTY(int scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY scaleFactorChanged)
    Q_PROPERTY(QSize hotSpotSize READ hotSpotSize WRITE setHotSpotSize NOTIFY hotSpotSizeChanged)
    Q_PROPERTY(quint64 hotSpotThreshold READ hotSpotThreshold WRITE setHotSpotThreshold NOTIFY hotSpotThresholdChanged)
    Q_PROPERTY(quint64 hotSpotPushTime READ hotSpotPushTime WRITE setHotSpotPushTime NOTIFY hotSpotPushTimeChanged)
    Q_PROPERTY(OutputWindow *window READ window CONSTANT)
    Q_ENUMS(Subpixel Transform HotSpot)
public:
    enum Subpixel {
      SubpixelUnknown = 0,
      SubpixelNone,
      SubpixelHorizontalRgb,
      SubpixelHorizontalBgr,
      SubpixelVerticalRgb,
      SubpixelVerticalBgr
    };

    enum Transform {
        TransformNormal = 0,
        Transform90,
        Transform180,
        Transform270,
        TransformFlipped,
        TransformFlipped90,
        TransformFlipped180,
        TransformFlipped270
    };

    enum ModeFlag {
        CurrentMode = 1,
        PreferredMode
    };
    Q_DECLARE_FLAGS(ModeFlags, ModeFlag)

    struct Mode {
        QSize size = QSize();
        int refreshRate = 60000;
        ModeFlags flags;
    };

    enum HotSpot {
        TopLeftHotSpot = 0,
        TopRightHotSpot,
        BottomLeftHotSpot,
        BottomRightHotSpot
    };

    Output(const QString &name, const QString &manufacturer,
           const QString &model);
    ~Output();

    QString name() const;
    QString manufacturer() const;
    QString model() const;

    int number() const;

    bool isPrimary() const;

    QPoint position() const;
    void setPosition(const QPoint &pt);

    QSize size() const;
    int refreshRate() const;

    QList<Mode> modes() const;

    void addMode(const QSize &size, ModeFlags flags = ModeFlags(),
                 int refreshRate = 60000);
    void setCurrentMode(const QSize &size, int refreshRate = 60000);

    QRect geometry() const;

    QRect availableGeometry() const;
    void setAvailableGeometry(const QRect &availableGeometry);

    QSize physicalSize() const;
    void setPhysicalSize(const QSize &size);

    Subpixel subpixel() const;
    void setSubpixel(const Subpixel &subpixel);

    Transform transform() const;
    void setTransform(const Transform &transform);

    int scaleFactor() const;
    void setScaleFactor(int scale);

    QSize hotSpotSize() const;
    void setHotSpotSize(const QSize &size);

    quint64 hotSpotThreshold() const;
    void setHotSpotThreshold(quint64 value);

    quint64 hotSpotPushTime() const;
    void setHotSpotPushTime(quint64 value);

    OutputWindow *window() const;

    QList<Surface *> surfaces() const;

    static Output *fromResource(wl_resource *resource);

    // Maps global coordinates to local space
    Q_INVOKABLE QPointF mapToOutput(const QPointF &pt);

    // Map output local coordinates to global space
    Q_INVOKABLE QPointF mapToGlobal(const QPointF &pt);

Q_SIGNALS:
    void primaryChanged();
    void positionChanged();
    void sizeChanged();
    void refreshRateChanged();
    void modesChanged();
    void currentModeChanged();
    void geometryChanged();
    void availableGeometryChanged();
    void physicalSizeChanged();
    void scaleFactorChanged();
    void subpixelChanged();
    void transformChanged();
    void hotSpotSizeChanged();
    void hotSpotThresholdChanged();
    void hotSpotPushTimeChanged();
    void hotSpotTriggered(HotSpot hotSpot);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_updateStarted())

    friend class FakeScreenBackend;
    friend class QuickSurfacePrivate;
    friend class NativeScreenBackend;
    friend class WlSurface;
};

} // namespace GreenIsland

#endif // GREENISLAND_OUTPUT_H
