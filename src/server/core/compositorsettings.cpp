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

#include <GreenIsland/QtWaylandCompositor/QWaylandCompositor>
#include <GreenIsland/QtWaylandCompositor/QWaylandInput>

#include "compositorsettings.h"
#include "compositorsettings_p.h"
#include "serverlogging_p.h"

namespace GreenIsland {

namespace Server {

/*
 * CompositorSettingsPrivate
 */

void CompositorSettingsPrivate::_q_setupKeymap()
{
    if (!compositor || !keymap || !initialized)
        return;

    QWaylandInputDevice *inputDevice = compositor->defaultInputDevice();
    if (inputDevice && inputDevice->keyboard()) {
        qCDebug(gLcCore,
                "Setting keymap to '%s' (variant '%s', options '%s', model '%s', rules '%s')",
                qPrintable(keymap->layout()),
                qPrintable(keymap->variant()),
                qPrintable(keymap->options()),
                qPrintable(keymap->model()),
                qPrintable(keymap->rules()));

        QWaylandKeymap wlKeymap(keymap->layout(), keymap->variant(),
                                keymap->options(), keymap->model(),
                                keymap->rules());
        inputDevice->keyboard()->setKeymap(wlKeymap);

        inputDevice->keyboard()->setRepeatRate(repeatRate);
        inputDevice->keyboard()->setRepeatDelay(repeatDelay);
    }
}

/*
 * CompositorSettings
 */

CompositorSettings::CompositorSettings(QObject *parent)
    : QObject(*new CompositorSettingsPrivate(), parent)
{
}

QWaylandCompositor *CompositorSettings::compositor() const
{
    Q_D(const CompositorSettings);
    return d->compositor;
}

void CompositorSettings::setCompositor(QWaylandCompositor *compositor)
{
    Q_D(CompositorSettings);

    if (d->compositor == compositor)
        return;

    if (d->compositor)
        disconnect(compositor, SIGNAL(defaultInputDeviceChanged()),
                   this, SLOT(_q_setupKeymap()));

    if (compositor) {
        connect(compositor, SIGNAL(defaultInputDeviceChanged()),
                this, SLOT(_q_setupKeymap()));
        d->_q_setupKeymap();
    }

    d->compositor = compositor;
    Q_EMIT compositorChanged();
}

Qt::KeyboardModifier CompositorSettings::windowActionKey() const
{
    Q_D(const CompositorSettings);
    return d->windowActionKey;
}

void CompositorSettings::setWindowActionKey(Qt::KeyboardModifier mod)
{
    Q_D(CompositorSettings);

    if (d->windowActionKey == mod)
        return;

    d->windowActionKey = mod;
    Q_EMIT windowActionKeyChanged();
}

quint32 CompositorSettings::keyboardRepeatRate() const
{
    Q_D(const CompositorSettings);
    return d->repeatRate;
}

void CompositorSettings::setKeyboardRepeatRate(quint32 rate)
{
    Q_D(CompositorSettings);

    if (d->repeatRate == rate)
        return;

    d->repeatRate = rate;
    Q_EMIT keyboardRepeatRateChanged();

    if (d->compositor) {
        QWaylandInputDevice *inputDevice = d->compositor->defaultInputDevice();
        if (inputDevice && inputDevice->keyboard())
            inputDevice->keyboard()->setRepeatRate(d->repeatRate);
    }
}

quint32 CompositorSettings::keyboardRepeatDelay() const
{
    Q_D(const CompositorSettings);
    return d->repeatDelay;
}

void CompositorSettings::setKeyboardRepeatDelay(quint32 delay)
{
    Q_D(CompositorSettings);

    if (d->repeatDelay == delay)
        return;

    d->repeatDelay = delay;
    Q_EMIT keyboardRepeatDelayChanged();

    if (d->compositor) {
        QWaylandInputDevice *inputDevice = d->compositor->defaultInputDevice();
        if (inputDevice && inputDevice->keyboard())
            inputDevice->keyboard()->setRepeatDelay(d->repeatDelay);
    }
}

Keymap *CompositorSettings::keymap() const
{
    Q_D(const CompositorSettings);
    return d->keymap;
}

void CompositorSettings::setKeymap(Keymap *keymap)
{
    Q_D(CompositorSettings);

    if (d->keymap == keymap)
        return;

    if (d->keymap) {
        disconnect(keymap, SIGNAL(rulesChanged()),
                   this, SLOT(_q_setupKeymap()));
        disconnect(keymap, SIGNAL(modelChanged()),
                   this, SLOT(_q_setupKeymap()));
        disconnect(keymap, SIGNAL(layoutChanged()),
                   this, SLOT(_q_setupKeymap()));
        disconnect(keymap, SIGNAL(variantChanged()),
                   this, SLOT(_q_setupKeymap()));
        disconnect(keymap, SIGNAL(optionsChanged()),
                   this, SLOT(_q_setupKeymap()));
    }

    if (keymap) {
        connect(keymap, SIGNAL(rulesChanged()),
                this, SLOT(_q_setupKeymap()));
        connect(keymap, SIGNAL(modelChanged()),
                this, SLOT(_q_setupKeymap()));
        connect(keymap, SIGNAL(layoutChanged()),
                this, SLOT(_q_setupKeymap()));
        connect(keymap, SIGNAL(variantChanged()),
                this, SLOT(_q_setupKeymap()));
        connect(keymap, SIGNAL(optionsChanged()),
                this, SLOT(_q_setupKeymap()));

        d->_q_setupKeymap();
    }

    d->keymap = keymap;
    Q_EMIT keymapChanged();
}

void CompositorSettings::classBegin()
{
}

void CompositorSettings::componentComplete()
{
    Q_D(CompositorSettings);
    d->initialized = true;
    d->_q_setupKeymap();
}

} // namespace Server

} // namespace GreenIsland

#include "moc_compositorsettings.cpp"
