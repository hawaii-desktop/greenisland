/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *               2015 Michael Spencer <sonrisesoftware@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *    Michael Spencer
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

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QScreen>
#include <QGuiApplication>

#include "screenconfiguration.h"

Q_LOGGING_CATEGORY(SCREENCONFIG, "greenisland.screenconfiguration")

namespace GreenIsland {

/*
 * ScreenOutput
 */

ScreenOutput::ScreenOutput(const QString &name, bool primary,
                           const QPoint &pos, const Mode &mode,
                           QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_primary(primary)
    , m_pos(pos)
    , m_mode(mode)
{
}

QString ScreenOutput::name() const
{
    return m_name;
}

bool ScreenOutput::isPrimary() const
{
    return m_primary;
}

void ScreenOutput::setPrimary(bool value)
{
    if (m_primary == value)
        return;

    m_primary = value;
    Q_EMIT primaryChanged();
}

QPoint ScreenOutput::position() const
{
    return m_pos;
}

void ScreenOutput::setPosition(const QPoint &pos)
{
    if (m_pos == pos)
        return;

    m_pos = pos;
    Q_EMIT positionChanged();
}

QRect ScreenOutput::geometry() const
{
    return QRect(m_pos, m_mode.size);
}

ScreenOutput::Mode ScreenOutput::mode() const
{
    return m_mode;
}

void ScreenOutput::setMode(const Mode &mode)
{
    if (m_mode.size == mode.size && m_mode.refreshRate == mode.refreshRate)
        return;

    m_mode = mode;
    Q_EMIT modeChanged();
}

Qt::ScreenOrientation ScreenOutput::orientation() const
{
    return m_orientation;
}

void ScreenOutput::setOrientation(Qt::ScreenOrientation orientation)
{
    if (m_orientation == orientation)
        return;

    m_orientation = orientation;
    Q_EMIT orientationChanged();
}

/*
 * ScreenConfiguration
 */

ScreenConfiguration::ScreenConfiguration(QObject *parent)
    : QObject(parent)
{
}

QList<ScreenOutput *> ScreenConfiguration::outputs() const
{
    return m_outputs;
}

ScreenConfiguration *ScreenConfiguration::parseJson(const QByteArray &data)
{
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qCWarning(SCREENCONFIG) << "Invalid configuration file, not top-level JSON object";
        return Q_NULLPTR;
    }

    const QJsonObject object = doc.object();

    ScreenConfiguration *config = new ScreenConfiguration;
    bool primarySet = false;

    const QJsonArray outputs = object.value(QStringLiteral("outputs")).toArray();
    for (int i = 0; i < outputs.size(); i++) {
        const QVariantMap outputSettings = outputs.at(i).toObject().toVariantMap();
        qCDebug(SCREENCONFIG) << "Output settings:" << outputSettings;

        QString name = outputSettings.value(QStringLiteral("name")).toString();
        qCDebug(SCREENCONFIG) << "Output name:" << name;

        bool primary = outputSettings.value(QStringLiteral("primary")).toBool();
        if (primary)
            primarySet = true;
        qCDebug(SCREENCONFIG) << "Output primary:" << primary;

        const QVariantMap posValue = outputSettings.value(QStringLiteral("position")).toMap();
        int x = posValue.value(QStringLiteral("x")).toInt();
        int y = posValue.value(QStringLiteral("y")).toInt();
        QPoint pos(x, y);
        qCDebug(SCREENCONFIG) << "Output position:" << pos;

        const QVariantMap modeValue = outputSettings.value(QStringLiteral("mode")).toMap();
        const QVariantMap sizeValue = modeValue.value(QStringLiteral("size")).toMap();
        ScreenOutput::Mode mode;
        int w = sizeValue.value(QStringLiteral("width")).toInt();
        int h = sizeValue.value(QStringLiteral("height")).toInt();
        mode.size = QSize(w, h);
        mode.refreshRate = modeValue.value(QStringLiteral("refreshRate")).toInt();
        qCDebug(SCREENCONFIG) << "Output size:" << mode.size;
        qCDebug(SCREENCONFIG) << "Output refresh rate:" << mode.refreshRate;

        Qt::ScreenOrientation orientation =
                static_cast<Qt::ScreenOrientation>(outputSettings.value(QStringLiteral("orientation")).toInt());
        qCDebug(SCREENCONFIG) << "Output orientation:" << orientation;

        ScreenOutput *output = new ScreenOutput(name, primary, pos, mode, config);
        output->setOrientation(orientation);
        config->m_outputs.append(output);
    }

    // Assign primary role to the first output if it was not
    // explicitly set by the configuration
    if (!primarySet && config->m_outputs.size() > 0)
        config->m_outputs.at(0)->setPrimary(true);

    return config;
}

ScreenConfiguration *ScreenConfiguration::detectConfiguration()
{
    ScreenConfiguration *config = new ScreenConfiguration;

    Q_FOREACH(QScreen *screen, QGuiApplication::screens()) {
        ScreenOutput::Mode mode;
        mode.size = QSize(screen->availableGeometry().width(),
                screen->availableGeometry().height());
        mode.refreshRate = screen->refreshRate() * 1000;

        QPoint pos(screen->availableGeometry().x(), screen->availableGeometry().y());
        bool isPrimary = screen == QGuiApplication::primaryScreen();

        ScreenOutput *output = new ScreenOutput(screen->name(), isPrimary, pos, mode, config);
        output->setOrientation(screen->orientation());
        config->m_outputs.append(output);
    }

    return config;
}

}

#include "moc_screenconfiguration.cpp"
