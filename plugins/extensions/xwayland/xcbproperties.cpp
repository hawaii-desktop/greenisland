/****************************************************************************
 * This file is part of Hawaii.
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

#include "xcbproperties.h"
#include "xcbresources.h"
#include "xcbwrapper.h"

namespace Xcb {

namespace Properties {

void dump(xcb_atom_t property, xcb_get_property_reply_t *reply)
{
    if (!reply)
        return;

    qint32 *incrValue;
    const char *textValue, *name;
    int len;
    xcb_atom_t *atomValue;

    switch (reply->type) {
    case Xcb::resources()->atoms->incr:
        incrValue = static_cast<qint32 *>(xcb_get_property_value(reply));
        break;
    case Xcb::resources()->atoms->utf8_string:
    case Xcb::resources()->atoms->string:
        if (reply->value_len > 40)
            len = 40;
        else
            len = reply->value_len;
        break;
    case XCB_ATOM_ATOM:
        atomValue = xcb_get_property_value(reply);
        for (int i = 0; i < reply->value_len; i++) {
            name = Xcb::Atom::nameFromAtom(atomValue[i]);
        }
}

} // namespace Cursors

} // namespace Xcb
