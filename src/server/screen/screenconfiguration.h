/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef SCREENCONFIGURATION_H
#define SCREENCONFIGURATION_H

#include <QtCore/QLoggingCategory>
#include <QtGui/QScreen>

Q_DECLARE_LOGGING_CATEGORY(SCREENCONFIG)

namespace GreenIsland {

class ScreenOutput : public QObject
{
    Q_OBJECT
public:
    struct Mode {
        QSize size;
        int refreshRate;
    };

    ScreenOutput(const QString &name, bool primary,
                 const QPoint &pos, const Mode &mode,
                 QObject *parent = 0);

    QString name() const;

    bool isPrimary() const;
    void setPrimary(bool value);

    QPoint position() const;
    void setPosition(const QPoint &pos);

    QRect geometry() const;

    Mode mode() const;
    void setMode(const Mode &mode);

    Qt::ScreenOrientation orientation() const;
    void setOrientation(Qt::ScreenOrientation orientation);

Q_SIGNALS:
    void primaryChanged();
    void positionChanged();
    void modeChanged();
    void orientationChanged();

private:
    QString m_name;
    bool m_primary;
    QPoint m_pos;
    Mode m_mode;
    Qt::ScreenOrientation m_orientation;
};

class ScreenConfiguration : public QObject
{
    Q_OBJECT
public:
    ScreenConfiguration(QObject *parent = 0);

    QList<ScreenOutput *> outputs() const;

    static ScreenConfiguration *parseJson(const QByteArray &data);

private:
    QList<ScreenOutput *> m_outputs;
};

}

#endif // SCREENCONFIGURATION_H
