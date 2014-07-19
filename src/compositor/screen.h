/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef SCREEN_H
#define SCREEN_H

#include <QtCore/QObject>

class ScreenPrivate;

class Screen : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool primary READ isPrimary NOTIFY primaryChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)
public:
    explicit Screen(QObject *parent = Q_NULLPTR);
    ~Screen();

    QString name() const;
    bool isPrimary() const;
    QRect geometry() const;

Q_SIGNALS:
    void nameChanged();
    void primaryChanged();
    void geometryChanged();

private:
    friend class FakeScreenBackend;
    friend class QScreenBackend;

    Q_DECLARE_PRIVATE(Screen)
    ScreenPrivate *const d_ptr;

    void setName(const QString &name);
    void setPrimary(bool value);
    void setGeometry(const QRect &rect);
};

#endif // SCREEN_H
