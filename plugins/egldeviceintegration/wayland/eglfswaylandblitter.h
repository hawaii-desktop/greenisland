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

#ifndef GREENISLAND_EGLFSWAYLANDBLITTER_H
#define GREENISLAND_EGLFSWAYLANDBLITTER_H

#include <QtGui/QOpenGLFunctions>

class QOpenGLShaderProgram;

namespace GreenIsland {

namespace Platform {

class EglFSWaylandContext;
class EglFSWaylandWindow;

class EglFSWaylandBlitter : public QOpenGLFunctions
{
public:
    EglFSWaylandBlitter(EglFSWaylandContext *context);
    ~EglFSWaylandBlitter();

    void blit(EglFSWaylandWindow *window);

private:
    EglFSWaylandContext *m_context;
    QOpenGLShaderProgram *m_blitProgram;
};

} // namespace Platform

} // namespace GreenIsland

#endif // GREENISLAND_EGLFSWAYLANDBLITTER_H
