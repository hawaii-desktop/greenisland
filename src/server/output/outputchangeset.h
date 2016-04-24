/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#ifndef GREENISLAND_OUTPUTCHANGESET_H
#define GREENISLAND_OUTPUTCHANGESET_H

#include <QtCore/QObject>

#include <GreenIsland/QtWaylandCompositor/QWaylandOutput>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class OutputChangesetPrivate;
class OutputConfigurationPrivate;

class GREENISLANDSERVER_EXPORT OutputChangeset : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(OutputChangeset)
    Q_PROPERTY(QWaylandOutput *output READ output CONSTANT)
    Q_PROPERTY(bool enabledChanged READ isEnabledChanged CONSTANT)
    Q_PROPERTY(bool primaryChanged READ isPrimaryChanged CONSTANT)
    Q_PROPERTY(bool transformChanged READ isTransformChanged CONSTANT)
    Q_PROPERTY(bool modeIdChanged READ isModeIdChanged CONSTANT)
    Q_PROPERTY(bool positionChanged READ isPositionChanged CONSTANT)
    Q_PROPERTY(bool scaleFactorChanged READ isScaleFactorChanged CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled CONSTANT)
    Q_PROPERTY(bool primary READ isPrimary CONSTANT)
    Q_PROPERTY(int modeId READ modeId CONSTANT)
    Q_PROPERTY(QWaylandOutput::Transform transform READ transform CONSTANT)
    Q_PROPERTY(QPoint position READ position CONSTANT)
    Q_PROPERTY(int scaleFactor READ scaleFactor CONSTANT)
public:
    QWaylandOutput *output() const;

    bool isEnabledChanged() const;
    bool isPrimaryChanged() const;
    bool isTransformChanged() const;
    bool isModeIdChanged() const;
    bool isPositionChanged() const;
    bool isScaleFactorChanged() const;

    bool isEnabled() const;
    bool isPrimary() const;
    int modeId() const;
    QWaylandOutput::Transform transform() const;
    QPoint position() const;
    int scaleFactor() const;

private:
    explicit OutputChangeset(QWaylandOutput *output, QObject *parent = Q_NULLPTR);

    friend class OutputConfigurationPrivate;
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_OUTPUTCHANGESET_H
