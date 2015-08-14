/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *    Giulio Camuffo
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

#ifndef GREENISLAND_ABSTRACTQUICKCOMPOSITOR_H
#define GREENISLAND_ABSTRACTQUICKCOMPOSITOR_H

#include "abstractcompositor.h"

class QQuickWindow;

namespace GreenIsland {

class AbstractQuickCompositorPrivate;
class AbstractOutput;
class SurfaceView;

class GREENISLAND_EXPORT AbstractQuickCompositor : public AbstractCompositor
{
public:
    AbstractQuickCompositor(const char *socketName = 0, AbstractCompositor::ExtensionFlags extensions = DefaultExtensions);

    SurfaceView *createView(Surface *surf) Q_DECL_OVERRIDE;
    AbstractOutput *createOutput(QWindow *window,
                                 const QString &manufacturer,
                                 const QString &model) Q_DECL_OVERRIDE;
};

} // namespace GreenIsland

#endif // GREENISLAND_ABSTRACTQUICKCOMPOSITOR_H
