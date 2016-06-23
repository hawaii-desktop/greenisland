/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (C) 2012-2013 Martin Gräßlin <mgraesslin@kde.org>
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

#ifndef XCB_WINDOW_H
#define XCB_WINDOW_H

#include <QtCore/QRect>

#include "xcbwrapper.h"

namespace Xcb {

/**
 * This class is an RAII wrapper for an xcb_window_t. An xcb_window_t hold by an instance of this class
 * will be freed when the instance gets destroyed.
 *
 * Furthermore the class provides wrappers around some xcb methods operating on an xcb_window_t.
 *
 * For the cases that one is more interested in wrapping the xcb methods the constructor which takes
 * an existing window and the @link reset method allow to disable the RAII functionality.
 **/
class Window
{
public:
    /**
     * Takes over responsibility of @p window. If @p window is not provided an invalid Window is
     * created. Use @link create to set an xcb_window_t later on.
     *
     * If @p destroy is @c true the window will be destroyed together with this object, if @c false
     * the window will be kept around. This is useful if you are not interested in the RAII capabilities
     * but still want to use a window like an object.
     *
     * @param window The window to manage.
     * @param destroy Whether the window should be destroyed together with the object.
     * @see reset
     **/
    Window(xcb_window_t window = XCB_WINDOW_NONE, bool destroy = true);
    /**
     * Creates an xcb_window_t and manages it. It's a convenient method to create a window with
     * depth, class and visual being copied from parent and border being @c 0.
     * @param geometry The geometry for the window to be created
     * @param mask The mask for the values
     * @param values The values to be passed to xcb_create_window
     * @param parent The parent window
     **/
    Window(const QRect &geometry, uint32_t mask = 0,
           const uint32_t *values = Q_NULLPTR, xcb_window_t parent = rootWindow());
    /**
     * Creates an xcb_window_t and manages it. It's a convenient method to create a window with
     * depth and visual being copied from parent and border being @c 0.
     * @param geometry The geometry for the window to be created
     * @param class The window class
     * @param mask The mask for the values
     * @param values The values to be passed to xcb_create_window
     * @param parent The parent window
     **/
    Window(const QRect &geometry, uint16_t windowClass,
           uint32_t mask = 0, const uint32_t *values = Q_NULLPTR,
           xcb_window_t parent = rootWindow());
    Window(const Window &other) = delete;
    ~Window();

    /**
     * Creates a new window for which the responsibility is taken over. If a window had been managed
     * before it is freed.
     *
     * Depth, class and visual are being copied from parent and border is @c 0.
     * @param geometry The geometry for the window to be created
     * @param mask The mask for the values
     * @param values The values to be passed to xcb_create_window
     * @param parent The parent window
     **/
    void create(const QRect &geometry, uint32_t mask = 0,
                const uint32_t *values = Q_NULLPTR,
                xcb_window_t parent = rootWindow());
    /**
     * Creates a new window for which the responsibility is taken over. If a window had been managed
     * before it is freed.
     *
     * Depth and visual are being copied from parent and border is @c 0.
     * @param geometry The geometry for the window to be created
     * @param class The window class
     * @param mask The mask for the values
     * @param values The values to be passed to xcb_create_window
     * @param parent The parent window
     **/
    void create(const QRect &geometry, uint16_t windowClass, uint32_t mask = 0,
                const uint32_t *values = Q_NULLPTR,
                xcb_window_t parent = rootWindow());
    /**
     * Frees the existing window and starts to manage the new @p window.
     * If @p destroy is @c true the new managed window will be destroyed together with this
     * object or when reset is called again. If @p destroy is @c false the window will not
     * be destroyed. It is then the responsibility of the caller to destroy the window.
     **/
    void reset(xcb_window_t window = XCB_WINDOW_NONE, bool destroy = true);
    /**
     * @returns @c true if a window is managed, @c false otherwise.
     **/
    bool isValid() const;
    /**
     * Configures the window with a new geometry.
     * @param geometry The new window geometry to be used
     **/
    void setGeometry(const QRect &geometry);
    void setGeometry(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void move(const QPoint &pos);
    void move(uint32_t x, uint32_t y);
    void resize(const QSize &size);
    void resize(uint32_t width, uint32_t height);
    void raise();
    void lower();
    void map();
    void unmap();
    void reparent(xcb_window_t parent, int x = 0, int y = 0);
    void changeProperty(xcb_atom_t property, xcb_atom_t type, uint8_t format, uint32_t lenght,
                        const void *data, uint8_t mode = XCB_PROP_MODE_REPLACE);
    void deleteProperty(xcb_atom_t property);
    void setBorderWidth(uint32_t width);
    void grabButton(uint8_t pointerMode, uint8_t keyboardmode,
                    uint16_t modifiers = XCB_MOD_MASK_ANY,
                    uint8_t button = XCB_BUTTON_INDEX_ANY,
                    uint16_t eventMask = XCB_EVENT_MASK_BUTTON_PRESS,
                    xcb_window_t confineTo = XCB_WINDOW_NONE,
                    xcb_cursor_t cursor = XCB_CURSOR_NONE,
                    bool ownerEvents = false);
    void ungrabButton(uint16_t modifiers = XCB_MOD_MASK_ANY, uint8_t button = XCB_BUTTON_INDEX_ANY);
    /**
     * Clears the window area. Same as xcb_clear_area with x, y, width, height being @c 0.
     **/
    void clear();
    void setBackgroundPixmap(xcb_pixmap_t pixmap);
    void defineCursor(xcb_cursor_t cursor);
    void focus(uint8_t revertTo = XCB_INPUT_FOCUS_POINTER_ROOT, xcb_timestamp_t time = XCB_TIME_CURRENT_TIME);
    void selectInput(uint32_t events);
    void kill();
    operator xcb_window_t() const;
    xcb_window_t window() const;

private:
    xcb_window_t doCreate(const QRect &geometry, uint16_t windowClass, uint32_t mask = 0,
                          const uint32_t *values = Q_NULLPTR,
                          xcb_window_t parent = rootWindow());
    void destroy();
    xcb_window_t m_window;
    bool m_destroy;
};

} // namespace Xcb

#endif // XCB_WINDOW_H
