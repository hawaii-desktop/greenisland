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

#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/private/qinputdevicemanager_p_p.h>

#include "libinput/libinputhandler.h"
#include "libinput/libinputmanager_p.h"

namespace GreenIsland {

namespace Platform {

LibInputManager::LibInputManager(QObject *parent)
    : QObject(parent)
    , m_handler(new LibInputHandler(this))
{
    QInputDeviceManager *inputManager =
            QGuiApplicationPrivate::inputDeviceManager();
    QInputDeviceManagerPrivate *inputManagerPriv
            = QInputDeviceManagerPrivate::get(inputManager);

    // Tell QPA about input devices
    connect(m_handler, &LibInputHandler::keyboardCountChanged, this,
            [this, inputManagerPriv](int count) {
        inputManagerPriv->setDeviceCount(QInputDeviceManager::DeviceTypeKeyboard,
                                         count);
    });
    connect(m_handler, &LibInputHandler::pointerCountChanged, this,
            [this, inputManagerPriv](int count) {
        inputManagerPriv->setDeviceCount(QInputDeviceManager::DeviceTypePointer,
                                         count);
    });
    connect(m_handler, &LibInputHandler::touchCountChanged, this,
            [this, inputManagerPriv](int count) {
        inputManagerPriv->setDeviceCount(QInputDeviceManager::DeviceTypeTouch,
                                         count);
    });
    connect(m_handler, &LibInputHandler::touchDeviceRegistered, this,
            [this](QTouchDevice *td) {
        QWindowSystemInterface::registerTouchDevice(td);
    });

    // Events
    connect(m_handler, &LibInputHandler::keyPressed, this,
            [this](const LibInputKeyEvent &e) {
        QWindowSystemInterface::handleExtendedKeyEvent(
                    Q_NULLPTR, QKeyEvent::KeyPress, e.key,
                    e.modifiers, e.nativeScanCode,
                    e.nativeVirtualKey, e.nativeModifiers,
                    e.text, e.autoRepeat, e.repeatCount);
    });
    connect(m_handler, &LibInputHandler::keyReleased, this,
            [this](const LibInputKeyEvent &e) {
        QWindowSystemInterface::handleExtendedKeyEvent(
                    Q_NULLPTR, QKeyEvent::KeyRelease, e.key,
                    e.modifiers, e.nativeScanCode,
                    e.nativeVirtualKey, e.nativeModifiers,
                    e.text, e.autoRepeat, e.repeatCount);
    });
    connect(m_handler, &LibInputHandler::mousePressed, this,
            [this](const LibInputMouseEvent &e) {
        QWindowSystemInterface::handleMouseEvent(
                    Q_NULLPTR, e.pos, e.pos, e.buttons,
                    e.modifiers);
    });
    connect(m_handler, &LibInputHandler::mouseReleased, this,
            [this](const LibInputMouseEvent &e) {
        QWindowSystemInterface::handleMouseEvent(
                    Q_NULLPTR, e.pos, e.pos, e.buttons,
                    e.modifiers);
    });
    connect(m_handler, &LibInputHandler::mouseMoved, this,
            [this](const LibInputMouseEvent &e) {
        QWindowSystemInterface::handleMouseEvent(
                    Q_NULLPTR, e.pos, e.pos, e.buttons,
                    e.modifiers);
    });
    connect(m_handler, &LibInputHandler::mouseWheel, this,
            [this](const LibInputMouseEvent &e) {
        QWindowSystemInterface::handleWheelEvent(
                    Q_NULLPTR, e.pos, e.pos,
                    e.wheelDelta, e.wheelOrientation,
                    e.modifiers);
    });
    connect(m_handler, &LibInputHandler::touchEvent, this,
            [this](const LibInputTouchEvent &e) {
        QWindowSystemInterface::handleTouchEvent(
                    Q_NULLPTR, e.device, e.touchPoints,
                    e.modifiers);
    });
    connect(m_handler, &LibInputHandler::touchCancel, this,
            [this](const LibInputTouchEvent &e) {
        QWindowSystemInterface::handleTouchCancelEvent(
                    Q_NULLPTR, e.device, e.modifiers);
    });

    // Change pointer coordinates when requested by QPA
    connect(inputManager, &QInputDeviceManager::cursorPositionChangeRequested, this,
            [this](const QPoint &pos) {
        m_handler->setPointerPosition(pos);
    });
}

LibInputHandler *LibInputManager::handler() const
{
    return m_handler;
}

} // namespace Platform

} // namespace GreenIsland

#include "moc_libinputmanager_p.cpp"
