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

#include <KScreen/Config>
#include <KScreen/ConfigMonitor>
#include <KScreen/Screen>

#include "compositor.h"
#include "output.h"
#include "screenmanager.h"
#include "surface.h"
#include "windowview.h"

static bool outputLess(KScreen::Output *a, KScreen::Output *b)
{
    return ((a->isEnabled() && !b->isEnabled())
            || (a->isEnabled() == b->isEnabled() && (a->isPrimary() && !b->isPrimary()))
            || (a->isPrimary() == b->isPrimary() && (a->pos().x() < b->pos().x()
                                                     || (a->pos().x() == b->pos().x() && a->pos().y() < b->pos().y()))));
}

/*
 * ScreenManagerPrivate
 */

class ScreenManagerPrivate
{
public:
    ScreenManagerPrivate(ScreenManager *self);

    Compositor *compositor;

    KScreen::Config *config;
    QList<Output *> outputs;

    QList<KScreen::Output *> sortOutputs(const QHash<int, KScreen::Output *> &outputs) const;

    void addOutput(KScreen::Output *output);
    void removeOutput(KScreen::Output *output);

    QList<QQuickItem *> findWindowsToMove(Output *removedOutput);
    void moveWindows(const QList<QQuickItem *> &items, const QRectF &removedGeometry);

    void _q_configurationChanged();
    void _q_outputAdded(KScreen::Output *output);
    void _q_outputRemoved(int id);
    void _q_outputEnabledChanged();

private:
    Q_DECLARE_PUBLIC(ScreenManager)
    ScreenManager *q_ptr;
};

ScreenManagerPrivate::ScreenManagerPrivate(ScreenManager *self)
    : q_ptr(self)
{
}

QList<KScreen::Output *> ScreenManagerPrivate::sortOutputs(const QHash<int, KScreen::Output *> &outputs) const
{
    QList<KScreen::Output *> ret = outputs.values();
    std::sort(ret.begin(), ret.end(), outputLess);
    return ret;
}

void ScreenManagerPrivate::addOutput(KScreen::Output *output)
{
    Q_Q(ScreenManager);

    // Create a new window for this output
    Output *customOutput = new Output(compositor, output);
    outputs.append(customOutput);
}

void ScreenManagerPrivate::removeOutput(KScreen::Output *output)
{
    Q_Q(ScreenManager);

    Output *outputFound = Q_NULLPTR;

    for (int i = 0; i < outputs.size(); i++) {
        if (outputs.at(i)->output() == output) {
            outputFound = outputs.at(i);
            break;
        }
    }

    // Remove output from the list and destroy it
    if (outputFound) {
        outputs.removeOne(outputFound);

        QList<QQuickItem *> items = findWindowsToMove(outputFound);
        QRectF geometry(outputFound->geometry());

        outputFound->deleteLater();

        moveWindows(items, geometry);
    }
}

QList<QQuickItem *> ScreenManagerPrivate::findWindowsToMove(Output *removedOutput)
{
    // The output is going to be removed this means that its
    // window and all its surface views will be gone, including
    // the window representations.
    // Since we create a view for each output we still have
    // a window representation for the remaining outputs,
    // all we have to do is to move the window representations
    // to the new primary output. However we won't be able to
    // access the removed output when it'll be delete so we do it
    // in two steps: this is step #1 where we determine which
    // window representations need to be moved

    QList<QQuickItem*> windowItems;

    // Return a list of window representations that need to be moved
    for (QWaylandSurface *surface: compositor->surfaces()) {
        for (QWaylandSurfaceView *surfaceView: surface->views()) {
            WindowView *view = static_cast<WindowView *>(surfaceView);
            if (!view || !view->output())
                continue;

            // This is a view for the removed output; its parent is the
            // window representation, add it to the list
            if (view->output() == removedOutput && view->parentItem())
                windowItems.append(view->parentItem());
        }
    }

    return windowItems;
}

void ScreenManagerPrivate::moveWindows(const QList<QQuickItem *> &items, const QRectF &removedGeometry)
{
    // First let's find the new primary output (the removed output
    // could have been the former primary output)
    Output *primaryOutput = Q_NULLPTR;

    for (QWaylandOutput *tmpOutput: compositor->outputs()) {
        Output *current = qobject_cast<Output *>(tmpOutput);
        if (!current)
            continue;

        if (current->isPrimary()) {
            primaryOutput = current;
            break;
        }
    }

    // Try with the first output in the list
    if (!primaryOutput)
        primaryOutput = qobject_cast<Output *>(compositor->outputs().at(0));
    if (!primaryOutput)
        return;

    // Move all window representation to the primary output
    for (QQuickItem *item: items) {
        // Get the surface view
        WindowView *view = item->property("child").value<WindowView *>();
        if (!view) {
            qWarning() << "Cannot cast child property of" << item;
            continue;
        }

        // Recalculate local coordinates
        qreal x = (item->x() * primaryOutput->geometry().width()) / removedGeometry.width();
        qreal y = (item->y() * primaryOutput->geometry().height()) / removedGeometry.height();
        item->setPosition(QPointF(x, y));

        // Set new global position
        view->surface()->setGlobalPosition(primaryOutput->mapToGlobal(QPointF(x, y)));
    }
}

void ScreenManagerPrivate::_q_configurationChanged()
{
    qWarning() << "TODO: Screen configuration changed";
}

void ScreenManagerPrivate::_q_outputAdded(KScreen::Output *output)
{
    addOutput(output);
}

void ScreenManagerPrivate::_q_outputRemoved(int id)
{
    removeOutput(config->outputs().value(id));
}

void ScreenManagerPrivate::_q_outputEnabledChanged()
{
    Q_Q(ScreenManager);

    KScreen::Output *output = qobject_cast<KScreen::Output *>(q->sender());

    if (output->isEnabled())
        addOutput(output);
    else
        removeOutput(output);
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
    d->config = KScreen::Config::current();
    Q_ASSERT(d->config);

    // Check whether the backend was loaded
    if (!d->config->loadBackend())
        qFatal("No KScreen backend found, aborting...");

    // Add configuration to the monitor
    KScreen::ConfigMonitor::instance()->addConfig(d->config);

    // Connect signals
    connect(KScreen::ConfigMonitor::instance(), SIGNAL(configurationChanged()),
            this, SLOT(_q_configurationChanged()));
    connect(d->config, SIGNAL(outputAdded(KScreen::Output*)),
            this, SLOT(_q_outputAdded(KScreen::Output*)));
    connect(d->config, SIGNAL(outputRemoved(int)),
            this, SLOT(_q_outputRemoved(int)));

    // Create outputs for the first time
    for (KScreen::Output *output: d->sortOutputs(d->config->outputs()))
        d->addOutput(output);
}

ScreenManager::~ScreenManager()
{
    delete d_ptr;
}

#include "moc_screenmanager.cpp"
