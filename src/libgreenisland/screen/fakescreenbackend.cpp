/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
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

#include "fakescreenbackend.h"
#include "screenconfiguration.h"

Q_LOGGING_CATEGORY(FAKE_BACKEND, "greenisland.screenbackend.fake")

namespace GreenIsland {

FakeScreenBackend::FakeScreenBackend(Compositor *compositor, QObject *parent)
    : ScreenBackend(compositor, parent)
    , m_config(Q_NULLPTR)
{
}

FakeScreenBackend::~FakeScreenBackend()
{
    QList<ScreenOutput *> outputs = m_outputMap.keys();
    Q_FOREACH (ScreenOutput *so, outputs)
        screenRemoved(so);

    if (m_config)
        m_config->deleteLater();
}

void FakeScreenBackend::loadConfiguration(const QString &fileName)
{
    qCDebug(FAKE_BACKEND) << "Load configuration from" << fileName;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(FAKE_BACKEND) << "Could not open configuration file"
                              << fileName << "for reading";
        return;
    }

    m_config = ScreenConfiguration::parseJson(file.readAll());

    file.close();
}

void FakeScreenBackend::acquireConfiguration()
{
    Q_FOREACH (ScreenOutput *output, m_config->outputs())
        screenAdded(output);

    Q_EMIT configurationAcquired();
}

void FakeScreenBackend::screenAdded(ScreenOutput *so)
{
    qCDebug(FAKE_BACKEND) << "Output added" << so->name() << so->geometry();

    if (m_outputMap.contains(so)) {
        qCWarning(FAKE_BACKEND) << "Output was already added!";
        return;
    }

    Output *output = new Output(compositor(), so->name(),
                                QStringLiteral("Green Island"),
                                so->name());
    output->setPrimary(so->isPrimary());
    output->setPosition(so->position());

    QWaylandOutputMode *mode =
            new QWaylandOutputMode(QStringLiteral("defaultMode"),
                                   so->mode().size,
                                   so->mode().refreshRate);
    output->setModes(QWaylandOutputModeList() << mode);

    switch (so->orientation()) {
    case Qt::PortraitOrientation:
        output->setTransform(QWaylandOutput::Transform90);
        break;
    case Qt::InvertedLandscapeOrientation:
        output->setTransform(QWaylandOutput::Transform180);
        break;
    case Qt::InvertedPortraitOrientation:
        output->setTransform(QWaylandOutput::Transform270);
        break;
    default:
        break;
    }

    m_outputMap[so] = output;

    Q_EMIT outputAdded(output);
}

void FakeScreenBackend::screenRemoved(ScreenOutput *so)
{
    qCDebug(FAKE_BACKEND) << "Output removed" << so->name() << so->geometry();

    if (!m_outputMap.contains(so)) {
        qCWarning(FAKE_BACKEND) << "Output not mapped!";
        return;
    }

    Output *output = m_outputMap[so];
    m_outputMap.remove(so);

    Q_EMIT outputRemoved(output);
    output->deleteLater();
}

}

#include "moc_fakescreenbackend.cpp"
