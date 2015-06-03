/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2012-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCompositor/QWaylandInputDevice>
#include <QtCompositor/QtCompositorVersion>

#include "compositorsettings.h"

namespace GreenIsland {

class CompositorSettingsPrivate
{
public:
    CompositorSettingsPrivate()
    {
    }

    void setKeymapLayout(const QString &layout)
    {
        QWaylandKeymap newKeymap(layout, keymap.variant(), keymap.options(),
                                 keymap.model(), keymap.rules());
        keymap = newKeymap;
    }

    void setKeymapVariant(const QString &variant)
    {
        QWaylandKeymap newKeymap(keymap.layout(), variant, keymap.options(),
                                 keymap.model(), keymap.rules());
        keymap = newKeymap;
    }

    void setKeymapOptions(const QString &options)
    {
        QWaylandKeymap newKeymap(keymap.layout(), keymap.variant(), options,
                                 keymap.model(), keymap.rules());
        keymap = newKeymap;
    }

    void setKeymapModel(const QString &model)
    {
        QWaylandKeymap newKeymap(keymap.layout(), keymap.variant(), keymap.options(),
                                 model, keymap.rules());
        keymap = newKeymap;
    }

    void setKeymapRules(const QString &rules)
    {
        QWaylandKeymap newKeymap(keymap.layout(), keymap.variant(), keymap.options(),
                                 keymap.model(), rules);
        keymap = newKeymap;
    }

    Compositor *compositor;
    Qt::KeyboardModifier windowActionKey;
    quint32 defaultKeyboardRate = { 40 };
    quint32 defaultKeyboardDelay = { 400 };
    QWaylandKeymap keymap;
};

CompositorSettings::CompositorSettings(Compositor *compositor)
    : QObject(compositor)
    , d_ptr(new CompositorSettingsPrivate())
{
    qRegisterMetaType<CompositorSettings *>("CompositorSettings*");

    Q_D(CompositorSettings);
    d->compositor = compositor;
    d->windowActionKey = Qt::MetaModifier;
}

CompositorSettings::~CompositorSettings()
{
    delete d_ptr;
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

#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return d->compositor->defaultInputDevice()->keyboardRepeatRate();
#else
    return d->defaultKeyboardRate;
#endif
}

void CompositorSettings::setKeyboardRepeatRate(quint32 rate)
{
#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    Q_D(CompositorSettings);

    if (keyboardRepeatRate() == rate)
        return;

    d->compositor->defaultInputDevice()->setKeyboardRepeatRate(rate);
    Q_EMIT keyboardRepeatRateChanged();
#else
    Q_UNUSED(rate)
#endif
}

quint32 CompositorSettings::keyboardRepeatDelay() const
{
    Q_D(const CompositorSettings);

#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return d->compositor->defaultInputDevice()->keyboardRepeatDelay();
#else
    return d->defaultKeyboardDelay;
#endif
}

void CompositorSettings::setKeyboardRepeatDelay(quint32 delay)
{
#if QTCOMPOSITOR_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    Q_D(CompositorSettings);

    if (keyboardRepeatDelay() == delay)
        return;

    d->compositor->defaultInputDevice()->setKeyboardRepeatDelay(delay);
    Q_EMIT keyboardRepeatDelayChanged();
#else
    Q_UNUSED(delay)
#endif
}

QString CompositorSettings::keyboardLayout() const
{
    Q_D(const CompositorSettings);
    return d->keymap.layout();
}

void CompositorSettings::setKeyboardLayout(const QString &layout)
{
    Q_D(CompositorSettings);

    if (d->keymap.layout() == layout)
        return;

    d->setKeymapLayout(layout);
    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

QString CompositorSettings::keyboardVariant() const
{
    Q_D(const CompositorSettings);
    return d->keymap.variant();
}

void CompositorSettings::setKeyboardVariant(const QString &variant)
{
    Q_D(CompositorSettings);

    if (d->keymap.variant() == variant)
        return;

    d->setKeymapVariant(variant);
    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

QString CompositorSettings::keyboardOptions() const
{
    Q_D(const CompositorSettings);
    return d->keymap.options();
}

void CompositorSettings::setKeyboardOptions(const QString &options)
{
    Q_D(CompositorSettings);

    if (d->keymap.options() == options)
        return;

    d->setKeymapOptions(options);
    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

QString CompositorSettings::keyboardRules() const
{
    Q_D(const CompositorSettings);
    return d->keymap.rules();
}

void CompositorSettings::setKeyboardRules(const QString &rules)
{
    Q_D(CompositorSettings);

    if (d->keymap.rules() == rules)
        return;

    d->setKeymapRules(rules);
    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

QString CompositorSettings::keyboardModel() const
{
    Q_D(const CompositorSettings);
    return d->keymap.model();
}

void CompositorSettings::setKeyboardModel(const QString &model)
{
    Q_D(CompositorSettings);

    if (d->keymap.model() == model)
        return;

    d->setKeymapModel(model);
    d->compositor->defaultInputDevice()->setKeymap(d->keymap);
    Q_EMIT keyMapChanged();
}

}

#include "moc_compositorsettings.cpp"
