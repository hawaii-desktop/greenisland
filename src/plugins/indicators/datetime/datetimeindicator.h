/****************************************************************************
 * This file is part of Indicators.
 *
 * Copyright (c) Pier Luigi Fiorini
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

#ifndef DATETIMEINDICATOR_H
#define DATETIMEINDICATOR_H

#include <QDateTime>
#include <QBasicTimer>

#include <VIndicator>

class QWindow;

class DateTimeView;

class GREENISLAND_EXPORT DateTimeIndicator : public VIndicator
{
    Q_OBJECT
public:
    explicit DateTimeIndicator();
    ~DateTimeIndicator();

    void startTimer();
    void stopTimer();

    QQuickItem *item() const;

public slots:
    void updateLabel();

protected:
    void timerEvent(QTimerEvent *);

private:
    bool m_showClock;
    bool m_showSeconds;
    bool m_showDayAndMonth;
    bool m_showWeekday;

    QWindow *m_window;
    DateTimeView *m_view;

    QTime m_time;
    QBasicTimer m_timer;
};

#endif // DATETIMEINDICATOR_H
