/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef GREENISLANDCOMPOSITOR_H
#define GREENISLANDCOMPOSITOR_H

#include <QtQuick/QQuickView>
#include <QtCompositor/QWaylandCompositor>

#include <GreenIsland/GreenIslandGlobal>

namespace GreenIsland {

class GREENISLAND_EXPORT Compositor : public QQuickView, public QWaylandCompositor
{
    Q_OBJECT
public:
    explicit Compositor(const char *socketName = 0,
                        QWaylandCompositor::ExtensionFlag extensions = QWaylandCompositor::DefaultExtensions);
    ~Compositor();

    void showGraphicsInfo();

    virtual void runShell();
    virtual void closeShell();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void logExtensions(const QString &label, const QString &extensions);
};

}

#endif // GREENISLANDCOMPOSITOR_H
