/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2014-2016 Pier Luigi Fiorini
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

#ifndef KEYBINDINGSFILTER_H
#define KEYBINDINGSFILTER_H

#include <QtCore/QPointer>
#include <QtQuick/QQuickItem>

#include "quickkeybindings.h"

class KeyBindingsFilter : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QuickKeyBindings *keyBindings READ keyBindings WRITE setKeyBindings NOTIFY keyBindingsChanged)
public:
    KeyBindingsFilter(QQuickItem *parent = Q_NULLPTR);

    QuickKeyBindings *keyBindings() const;
    void setKeyBindings(QuickKeyBindings *keyBindings);

Q_SIGNALS:
    void keyBindingsChanged();

protected:
    bool eventFilter(QObject *, QEvent *);

private:
    QPointer<QQuickWindow> m_window;
    QPointer<QuickKeyBindings> m_keyBindings;
};

#endif // KEYBINDINGSFILTER_H
