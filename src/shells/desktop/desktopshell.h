/****************************************************************************
 * This file is part of Desktop Shell.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
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

#ifndef DESKTOPSHELL_H
#define DESKTOPSHELL_H

#include <QRect>

#include <VShell>

class PanelView;

class DesktopShell : public VShell
{
    Q_OBJECT
    Q_PROPERTY(QRectF availableGeometry READ availableGeometry WRITE setAvailableGeometry NOTIFY availableGeometryChanged)
public:
    explicit DesktopShell();
    ~DesktopShell();

    QRectF screenGeometry() const;

    QRectF availableGeometry() const;
    void setAvailableGeometry(const QRectF &rect);

    void show();
    void hide();

signals:
    void availableGeometryChanged();

private:
    QRectF m_availableGeometry;
    PanelView *m_panelView;
};

#endif // DESKTOPSHELL_H
