/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
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
