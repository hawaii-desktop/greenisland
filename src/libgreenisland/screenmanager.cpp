/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCore/QRect>
#include <QtCore/QTimer>
#include <QtCompositor/private/qwlcompositor_p.h>

#include <KScreen/Config>
#include <KScreen/ConfigMonitor>
#include <KScreen/GetConfigOperation>
#include <KScreen/Screen>

#include "compositor.h"
#include "output.h"
#include "outputwindow.h"
#include "quicksurface.h"
#include "screenmanager.h"
#include "windowview.h"

static bool outputLess(KScreen::OutputPtr &a, KScreen::OutputPtr &b)
{
    return ((a->isEnabled() && !b->isEnabled())
            || (a->isEnabled() == b->isEnabled() && (a->isPrimary() && !b->isPrimary()))
            || (a->isPrimary() == b->isPrimary() && (a->pos().x() < b->pos().x()
                                                     || (a->pos().x() == b->pos().x() && a->pos().y() < b->pos().y()))));
}

static QList<KScreen::OutputPtr> sortOutputs(const KScreen::OutputList &outputs)
{
    QList<KScreen::OutputPtr> ret = outputs.values();
    std::sort(ret.begin(), ret.end(), outputLess);
    return ret;
}

namespace GreenIsland {

/*
 * ScreenManagerPrivate
 */

class ScreenManagerPrivate
{
public:
    ScreenManagerPrivate(ScreenManager *self);

    Compositor *compositor;

    KScreen::ConfigPtr config;

    void addOutput(const KScreen::OutputPtr &output);
    void removeOutput(const KScreen::OutputPtr &output);

    void _q_configurationChanged();
    void _q_outputAdded(const KScreen::OutputPtr &output);
    void _q_outputRemoved(int id);

private:
    Q_DECLARE_PUBLIC(ScreenManager)
    ScreenManager *q_ptr;
};

ScreenManagerPrivate::ScreenManagerPrivate(ScreenManager *self)
    : q_ptr(self)
{
}

void ScreenManagerPrivate::addOutput(const KScreen::OutputPtr &output)
{
    Q_Q(ScreenManager);

    // Create a new window for this output
    (void)new Output(compositor, output);
}

void ScreenManagerPrivate::removeOutput(const KScreen::OutputPtr &output)
{
    Q_Q(ScreenManager);

    Output *outputFound = Q_NULLPTR;

    // Find the output that matches KScreen's
    for (QWaylandOutput *curOutput: compositor->outputs()) {
        Output *customOutput = qobject_cast<Output *>(curOutput);
        if (!customOutput)
            continue;
        if (customOutput->output() == output) {
            outputFound = customOutput;
            break;
        }
    }
    if (!outputFound)
        return;

    // Find new primary output
    Output *primaryOutput = qobject_cast<Output *>(compositor->primaryOutput());
    if (!primaryOutput)
        return;

    // Remove surface views and window representations of this output
    for (QWaylandSurface *surface: outputFound->surfaces()) {
        for (QWaylandSurfaceView *surfaceView: surface->views()) {
            WindowView *view = static_cast<WindowView *>(surfaceView);
            if (!view || view->output() != outputFound || !view->parentItem())
                continue;

            QQuickItem *item = view->parentItem();
            QRectF removedGeometry(outputFound->geometry());

            // Recalculate local coordinates
            qreal x = (item->x() * primaryOutput->geometry().width()) / removedGeometry.width();
            qreal y = (item->y() * primaryOutput->geometry().height()) / removedGeometry.height();
            item->setPosition(QPointF(x, y));

            // Set new global position
            view->surface()->setGlobalPosition(primaryOutput->mapToGlobal(QPointF(x, y)));
        }
    }

    // Delete window and output
    outputFound->window()->deleteLater();
    outputFound->deleteLater();
}

void ScreenManagerPrivate::_q_configurationChanged()
{
    qWarning() << "TODO: Screen configuration changed";
}

void ScreenManagerPrivate::_q_outputAdded(const KScreen::OutputPtr &output)
{
    addOutput(output);
}

void ScreenManagerPrivate::_q_outputRemoved(int id)
{
    removeOutput(config->outputs().value(id));
}

/*
 * ScreenManager
 */

ScreenManager::ScreenManager(Compositor *compositor)
    : QObject(compositor)
    , d_ptr(new ScreenManagerPrivate(this))
{
    Q_D(ScreenManager);

    // Save compositor pointer
    d->compositor = compositor;

    // Load current configuration
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, [=](KScreen::ConfigOperation *op) {
        d->config = qobject_cast<KScreen::GetConfigOperation *>(op)->config();
        Q_ASSERT(!d->config.isNull());
        qDebug() << "Screen configuration acquired";

        // Monitor configuration
        KScreen::ConfigMonitor::instance()->addConfig(d->config);

        // Create outputs for the first time
        for (const KScreen::OutputPtr &output: sortOutputs(d->config->outputs()))
            d->addOutput(output);

        // Connect configuration signals
        connect(d->config.data(), SIGNAL(outputAdded(KScreen::OutputPtr)),
                this, SLOT(_q_outputAdded(KScreen::OutputPtr)));
        connect(d->config.data(), SIGNAL(outputRemoved(int)),
                this, SLOT(_q_outputRemoved(int)));
    });

    // Connect signals
    connect(KScreen::ConfigMonitor::instance(), SIGNAL(configurationChanged()),
            this, SLOT(_q_configurationChanged()));
}

ScreenManager::~ScreenManager()
{
    delete d_ptr;
}

}

#include "moc_screenmanager.cpp"
