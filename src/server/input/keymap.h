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

#ifndef GREENISLAND_KEYMAP_H
#define GREENISLAND_KEYMAP_H

#include <QtCore/QObject>

#include <GreenIsland/server/greenislandserver_export.h>

namespace GreenIsland {

namespace Server {

class KeymapPrivate;

class GREENISLANDSERVER_EXPORT Keymap : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Keymap)
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    Q_PROPERTY(QString variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(QString options READ options WRITE setOptions NOTIFY optionsChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString rules READ rules WRITE setRules NOTIFY rulesChanged)
public:
    Keymap(QObject *parent = Q_NULLPTR);

    QString layout() const;
    void setLayout(const QString &layout);

    QString variant() const;
    void setVariant(const QString &variant);

    QString options() const;
    void setOptions(const QString &options);

    QString model() const;
    void setModel(const QString &model);

    QString rules() const;
    void setRules(const QString &rules);

Q_SIGNALS:
    void layoutChanged();
    void variantChanged();
    void optionsChanged();
    void modelChanged();
    void rulesChanged();
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_KEYMAP_H
