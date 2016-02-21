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

#ifndef GREENISLAND_COMPOSITORSETTINGS_H
#define GREENISLAND_COMPOSITORSETTINGS_H

#include <QtQml/QQmlParserStatus>

#include <GreenIsland/Server/Keymap>

class QWaylandCompositor;

namespace GreenIsland {

namespace Server {

class CompositorSettingsPrivate;

class GREENISLANDSERVER_EXPORT CompositorSettings : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_DECLARE_PRIVATE(CompositorSettings)
    Q_PROPERTY(QWaylandCompositor *compositor READ compositor WRITE setCompositor NOTIFY compositorChanged)
    Q_PROPERTY(Qt::KeyboardModifier windowActionKey READ windowActionKey WRITE setWindowActionKey NOTIFY windowActionKeyChanged)
    Q_PROPERTY(quint32 keyboardRepeatRate READ keyboardRepeatRate WRITE setKeyboardRepeatRate NOTIFY keyboardRepeatRateChanged)
    Q_PROPERTY(quint32 keyboardRepeatDelay READ keyboardRepeatDelay WRITE setKeyboardRepeatDelay NOTIFY keyboardRepeatDelayChanged)
    Q_PROPERTY(Keymap *keymap READ keymap WRITE setKeymap NOTIFY keymapChanged)
public:
    CompositorSettings(QObject *parent = Q_NULLPTR);

    QWaylandCompositor *compositor() const;
    void setCompositor(QWaylandCompositor *compositor);

    Qt::KeyboardModifier windowActionKey() const;
    void setWindowActionKey(Qt::KeyboardModifier mod);

    quint32 keyboardRepeatRate() const;
    void setKeyboardRepeatRate(quint32 rate);

    quint32 keyboardRepeatDelay() const;
    void setKeyboardRepeatDelay(quint32 delay);

    Keymap *keymap() const;
    void setKeymap(Keymap *keymap);

Q_SIGNALS:
    void compositorChanged();
    void windowActionKeyChanged();
    void keyboardRepeatRateChanged();
    void keyboardRepeatDelayChanged();
    void keymapChanged();

protected:
    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_setupKeymap())
};

} // namespace Server

} // namespace GreenIsland

#endif // GREENISLAND_COMPOSITORSETTINGS_H
