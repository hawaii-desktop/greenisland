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

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QStandardPaths>
#include <QStringBuilder>

#include <VBookmarkManager>
#include <VSettings>

#include "launchermodel.h"
#include "launcherapplication.h"
#include "launcherurl.h"
#include "cmakedirs.h"

LauncherModel::LauncherModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Settings
    m_settings = new VSettings("org.hawaii.greenisland");

    // Load pinned applications
    QStringList pinnedApps = m_settings->value("launcher/pinned-apps").toStringList();
    pinnedApps = QStringList() << "swordfish.desktop" << "hawaii-terminal.desktop" << "hawaii-system-preferences.desktop";
    foreach(QString file, pinnedApps) {
        QString fileFound = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, file);
        QFileInfo fileInfo(fileFound);
        if (fileInfo.exists())
            pinApplication(fileInfo.filePath());
    }
}

LauncherModel::~LauncherModel()
{
    delete m_settings;
}

QVariant LauncherModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);

    if (!index.isValid())
        return QVariant();

    LauncherItem *item = m_list.at(index.row());
    if (item->type() == LauncherItem::ApplicationLauncherItem)
        return QVariant::fromValue(qobject_cast<LauncherApplication *>(item));
    else
        return QVariant::fromValue(qobject_cast<LauncherUrl *>(item));
}

int LauncherModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_list.size();
}

void LauncherModel::pinApplication(const QString &path)
{
    QFileInfo fileInfo(path);
    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(new LauncherApplication(path));
    endInsertRows();
}

void LauncherModel::pinUrl(const QUrl &url)
{
}

void LauncherModel::addItem(LauncherApplication *item)
{
    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(item);
    endInsertRows();
}

LauncherApplication *LauncherModel::findItem(const QString &id, const QString &ver)
{
    foreach(LauncherItem * item, m_list) {
        LauncherApplication *app = qobject_cast<LauncherApplication *>(item);
        if (app && app->identifier() == id && app->version() == ver)
            return app;
    }

    return 0;
}

#include "moc_launchermodel.cpp"
