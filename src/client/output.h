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

#ifndef GREENISLANDCLIENT_OUTPUT_H
#define GREENISLANDCLIENT_OUTPUT_H

#include <QtCore/QObject>
#include <QtCore/QRect>

#include <GreenIsland/client/greenislandclient_export.h>

class QScreen;

namespace GreenIsland {

namespace Client {

class OutputPrivate;
class Registry;

class GREENISLANDCLIENT_EXPORT Output : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Output)
    Q_PROPERTY(QString manufacturer READ manufacturer NOTIFY manufacturerChanged)
    Q_PROPERTY(QString model READ model NOTIFY modelChanged)
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)
    Q_PROPERTY(qreal refreshRate READ refreshRate NOTIFY refreshRateChanged)
    Q_PROPERTY(QSize physicalSize READ physicalSize NOTIFY physicalSizeChanged)
    Q_PROPERTY(qint32 scale READ scale NOTIFY scaleChanged)
    Q_PROPERTY(Subpixel subpixel READ subpixel NOTIFY subpixelChanged)
    Q_PROPERTY(Transform transform READ transform NOTIFY transformChanged)
public:
    enum Subpixel {
      SubpixelUnknown = 0,
      SubpixelNone,
      SubpixelHorizontalRgb,
      SubpixelHorizontalBgr,
      SubpixelVerticalRgb,
      SubpixelVerticalBgr
    };
    Q_ENUM(Subpixel)

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
    Q_ENUM(Transform)

    enum ModeFlag {
        CurrentMode = 1,
        PreferredMode
    };
    Q_ENUM(ModeFlag)
    Q_DECLARE_FLAGS(ModeFlags, ModeFlag)

    struct Mode {
        QSize size;
        qreal refreshRate;
        ModeFlags flags;
    };

    QString manufacturer() const;
    QString model() const;
    QPoint position() const;
    QSize size() const;
    QRect geometry() const;
    qreal refreshRate() const;
    QSize physicalSize() const;
    qint32 scale() const;
    QList<Mode> modes() const;
    Subpixel subpixel() const;
    Transform transform() const;

    static Output *fromQt(QScreen *screen, QObject *parent = Q_NULLPTR);

    static QByteArray interfaceName();

Q_SIGNALS:
    void manufacturerChanged();
    void modelChanged();
    void positionChanged();
    void sizeChanged();
    void geometryChanged();
    void refreshRateChanged();
    void physicalSizeChanged();
    void scaleChanged();
    void subpixelChanged();
    void transformChanged();
    void outputChanged();
    void modeAdded(const GreenIsland::Client::Output::Mode &mode);
    void modeChanged(const GreenIsland::Client::Output::Mode &mode);

private:
    Output(QObject *parent = Q_NULLPTR);

    friend class Registry;
};

} // namespace Client

} // namespace GreenIsland

Q_DECLARE_METATYPE(GreenIsland::Client::Output::Mode)

#endif // GREENISLANDCLIENT_OUTPUT_H
