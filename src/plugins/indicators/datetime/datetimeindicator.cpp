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

#include <QWindow>

#include "datetimeindicator.h"
#include "datetimeview.h"

DateTimeIndicator::DateTimeIndicator()
    : VIndicator("datetime"),
      m_showClock(true),
      m_showSeconds(true),
      m_showDayAndMonth(false),
      m_showWeekday(false)
{
    // Create the view
    m_window = new QWindow();
    m_view = new DateTimeView(m_window);

    // Start the timer
    startTimer();
    updateLabel();
}

DateTimeIndicator::~DateTimeIndicator()
{
    stopTimer();

    delete m_view;
    delete m_window;
}

void DateTimeIndicator::startTimer()
{
    if (!m_timer.isActive()) {
        m_time = QTime::currentTime();

        int time = m_showSeconds ? 1000 : 60000 - m_time.second() * 1000;
        m_timer.start(time, this);
    }
}

void DateTimeIndicator::stopTimer()
{
    m_timer.stop();
}

void DateTimeIndicator::showView()
{
    m_view->show();
}

void DateTimeIndicator::hideView()
{
    m_view->hide();
}

void DateTimeIndicator::updateLabel()
{
    QString dateTimeFormat("hh:mm");
    if (m_showSeconds)
        dateTimeFormat += QLatin1String(":ss");
    if (m_showDayAndMonth)
        dateTimeFormat = QLatin1String("MMM dd ") + dateTimeFormat;
    if (m_showWeekday)
        dateTimeFormat = QLatin1String("ddd ") + dateTimeFormat;

    setLabel(m_time.toString(dateTimeFormat));
}

void DateTimeIndicator::timerEvent(QTimerEvent *)
{
    QTime now = QTime::currentTime();

    if (now.second() == 59 && now.minute() == m_time.minute() && now.hour() == m_time.hour()) {
        m_time.addSecs(m_showSeconds ? 1 : 60);
        m_timer.start(m_showSeconds ? 1500 : 60500, this);
    } else {
        m_time = now;

        int time = m_showSeconds ? 1000 : 60000 - m_time.second() * 1000;
        m_timer.start(time, this);
    }

    updateLabel();
}

#include "moc_datetimeindicator.cpp"
