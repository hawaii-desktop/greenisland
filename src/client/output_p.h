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

#ifndef GREENISLANDCLIENT_OUTPUT_P_H
#define GREENISLANDCLIENT_OUTPUT_P_H

#include <QtCore/private/qobject_p.h>

#include <GreenIsland/Client/Output>
#include <GreenIsland/client/private/qwayland-wayland.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Green Island API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace GreenIsland {

namespace Client {

class GREENISLANDCLIENT_EXPORT OutputPrivate
        : public QObjectPrivate
        , public QtWayland::wl_output
{
    Q_DECLARE_PUBLIC(Output)
public:
    OutputPrivate();

    QString manufacturer;
    QString model;

    QPoint position;
    QSize physicalSize;
    qint32 scale;

    QVector<Output::Mode> modes;
    QVector<Output::Mode>::iterator currentMode;

    Output::Subpixel subpixelValue;
    Output::Transform transformValue;

    static Output *fromWlOutput(struct ::wl_output *output);
    static OutputPrivate *get(Output *output) { return output->d_func(); }

protected:
    void output_geometry(int32_t x, int32_t y,
                         int32_t physical_width, int32_t physical_height,
                         int32_t subpixel, const QString &make,
                         const QString &model, int32_t transform) Q_DECL_OVERRIDE;
    void output_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh) Q_DECL_OVERRIDE;
    void output_done() Q_DECL_OVERRIDE;
    void output_scale(int32_t factor) Q_DECL_OVERRIDE;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_OUTPUT_P_H
