/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include "fakescreenbackend.h"
#include "screenbackend_p.h"

Q_LOGGING_CATEGORY(FAKE_BACKEND, "greenisland.screenbackend.fake")

namespace GreenIsland {

namespace Server {

FakeScreenBackend::FakeScreenBackend(QObject *parent)
    : ScreenBackend(parent)
{
}

void FakeScreenBackend::setConfiguration(const QString &fileName)
{
    m_fileName = fileName;
}

void FakeScreenBackend::acquireConfiguration()
{
    qCDebug(FAKE_BACKEND) << "Load configuration from" << m_fileName;

    QFile file(m_fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(FAKE_BACKEND) << "Could not open configuration file"
                                << m_fileName << "for reading";
        return;
    }

    QByteArray data = file.readAll();

    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qCWarning(FAKE_BACKEND) << "Invalid configuration file, no top-level JSON object";
        return;
    }

    bool primarySet = false;

    const QJsonObject object = doc.object();
    const QJsonArray outputs = object.value(QStringLiteral("outputs")).toArray();

    for (int i = 0; i < outputs.size(); i++) {
        const QVariantMap outputSettings = outputs.at(i).toObject().toVariantMap();
        qCDebug(FAKE_BACKEND) << "Output settings:" << outputSettings;

        QString name = outputSettings.value(QStringLiteral("name")).toString();
        qCDebug(FAKE_BACKEND) << "Output name:" << name;

        bool primary = outputSettings.value(QStringLiteral("primary")).toBool();
        qCDebug(FAKE_BACKEND) << "Output primary:" << primary;

        const QVariantMap posValue = outputSettings.value(QStringLiteral("position")).toMap();
        int x = posValue.value(QStringLiteral("x")).toInt();
        int y = posValue.value(QStringLiteral("y")).toInt();
        QPoint pos(x, y);
        qCDebug(FAKE_BACKEND) << "Output position:" << pos;

        const QVariantMap modeValue = outputSettings.value(QStringLiteral("mode")).toMap();
        const QVariantMap sizeValue = modeValue.value(QStringLiteral("size")).toMap();
        int w = sizeValue.value(QStringLiteral("width")).toInt();
        int h = sizeValue.value(QStringLiteral("height")).toInt();
        QSize size = QSize(w, h);
        int refreshRate = modeValue.value(QStringLiteral("refreshRate")).toInt();
        qCDebug(FAKE_BACKEND) << "Output size:" << size;
        qCDebug(FAKE_BACKEND) << "Output refresh rate:" << refreshRate;

        QSize physicalSize = outputSettings.value(QStringLiteral("physicalSize"), QSize()).toSize();
        if (!physicalSize.isValid()) {
            physicalSize.setWidth(w * 0.26458);
            physicalSize.setHeight(h * 0.26458);
        }
        qCDebug(FAKE_BACKEND) << "Physical size millimiters:" << size;

        Qt::ScreenOrientation orientation =
                static_cast<Qt::ScreenOrientation>(outputSettings.value(QStringLiteral("orientation")).toInt());
        qCDebug(FAKE_BACKEND) << "Output orientation:" << orientation;

        Screen *screen = new Screen(this);
        ScreenPrivate *screenPrivate = Screen::get(screen);
        screenPrivate->m_manufacturer = QStringLiteral("Green Island");
        screenPrivate->m_model = name;
        if (physicalSize.isValid())
            screenPrivate->setPhysicalSize(physicalSize);
        screenPrivate->setPosition(pos);
        screenPrivate->setSize(size);
        screenPrivate->setRefreshRate(refreshRate);
        // TODO: How do we get the scale factor from QScreen?
        screenPrivate->setScaleFactor(1);

        switch (orientation) {
        case Qt::PortraitOrientation:
            screenPrivate->setTransform(Screen::Transform90);
            break;
        case Qt::InvertedLandscapeOrientation:
            screenPrivate->setTransform(Screen::Transform180);
            break;
        case Qt::InvertedPortraitOrientation:
            screenPrivate->setTransform(Screen::Transform270);
            break;
        default:
            break;
        }

        ScreenBackend::get(this)->screens.append(screen);
        Q_EMIT screenAdded(screen);

        if (primary) {
            primarySet = true;
            Q_EMIT primaryScreenChanged(screen);
        }
    }

    // Automatically promote to primary the first screen if the configuration
    // doesn't explicitely assign one
    if (!primarySet)
        Q_EMIT primaryScreenChanged(ScreenBackend::get(this)->screens.at(0));
}

} // namespace Server

} // namespace GreenIsland

#include "moc_fakescreenbackend.cpp"
