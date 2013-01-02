/****************************************************************************
 * This file is part of GreenIslandQmlPlugin.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL3$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
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

#include <QDir>
#include <QIcon>
#include <QPluginLoader>
#include <QStringList>

#include <VIndicator>
#include <VIndicatorPlugin>

#include "indicatorsmodel.h"
#include "cmakedirs.h"

IndicatorsModel::IndicatorsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // TODO: Load from settings
    QStringList indicators;
    indicators << "power" << "volume" << "datetime" << "usermenu";

    // Load plugins
    QDir pluginsDir(QStringLiteral("%1/greenisland/indicators").arg(INSTALL_PLUGINSDIR));
    foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        VIndicatorPlugin *plugin = qobject_cast<VIndicatorPlugin *>(
                                       loader.instance());
        if (!plugin)
            continue;

        foreach(QString key, plugin->keys()) {
            if (indicators.contains(key))
                loadPlugin(key, plugin);
        }
    }
}

QHash<int, QByteArray> IndicatorsModel::roleNames() const
{
    // Set roles
    QHash<int, QByteArray> roles;
    roles[IndicatorRole] = "indicator";
    return roles;
}

QVariant IndicatorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    VIndicator *indicator = m_indicators.at(index.row());

    switch (role) {
        case Qt::DecorationRole:
            return QIcon::fromTheme(indicator->iconName());
        case Qt::DisplayRole:
            return indicator->label();
        case IndicatorRole:
            return QVariant::fromValue(indicator);
        default:
            break;
    }

    return QVariant();
}

int IndicatorsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_indicators.size();
}

QModelIndex IndicatorsModel::indexForName(const QString &name) const
{
    for (int i = 0; i < m_indicators.size(); i++) {
        VIndicator *indicator = m_indicators.at(i);

        if (indicator->name() == name)
            return index(i, 0);
    }

    return QModelIndex();
}

void IndicatorsModel::loadPlugin(const QString &name,
                                 VIndicatorPlugin *plugin)
{
#if 0
    if (m_indicators.contains(name))
        return;
#endif

    // Create the indicator
    VIndicator *indicator = plugin->create(name);
    m_indicators << indicator;
}

#include "moc_indicatorsmodel.cpp"
