/****************************************************************************
 * This file is part of Indicators.
 *
 * Copyright (c) Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef POWERINDICATOR_H
#define POWERINDICATOR_H

#include <QStringList>
#include <VIndicator>

class QWindow;

class PowerView;

class GREENISLAND_EXPORT PowerIndicator : public VIndicator
{
    Q_OBJECT
public:
    explicit PowerIndicator();
    ~PowerIndicator();

    Q_INVOKABLE void showView();
    Q_INVOKABLE void hideView();

public slots:
    void updateLabel();

private:
    int m_index;
    QStringList m_list;
    QWindow *m_window;
    PowerView *m_view;
};

#endif // POWERINDICATOR_H
