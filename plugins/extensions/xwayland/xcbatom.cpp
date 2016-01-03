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

#include "xcbatom.h"
#include "xcbwrapper.h"

namespace Xcb {

Atom::Atom(const QByteArray &name, bool onlyIfExists)
    : m_name(name)
    , m_retrieved(false)
    , m_cookie(xcb_intern_atom(connection(), onlyIfExists, name.length(), name.constData()))
    , m_atom(XCB_ATOM_NONE)
{
}

Atom::Atom()
    : m_retrieved(false)
    , m_atom(XCB_ATOM_NONE)
{
}

Atom::~Atom()
{
    if (!m_retrieved && m_cookie.sequence)
        xcb_discard_reply(connection(), m_cookie.sequence);
}

bool Atom::isValid()
{
    getReply();
    return m_atom != XCB_ATOM_NONE;
}

Atom::operator xcb_atom_t() const
{
    const_cast<Atom *>(this)->getReply();
    return m_atom;
}

QString Atom::nameFromAtom(xcb_atom_t atom)
{
    if (atom == XCB_ATOM_NONE)
        return QStringLiteral("None");

    QString name;

    xcb_generic_error_t *e;
    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(connection(), atom);
    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(connection(), cookie, &e);
    if (reply) {
        int length = xcb_get_atom_name_name_length(reply);
        char *n = xcb_get_atom_name_name(reply);
        name = name.sprintf("%.*s", length, n);
    } else {
        name = name.sprintf("(atom %u)", atom);
    }

    free(reply);

    return name;
}

void Atom::getReply()
{
    if (m_retrieved || !m_cookie.sequence)
        return;

    xcb_intern_atom_reply_t *reply =
            xcb_intern_atom_reply(connection(), m_cookie, Q_NULLPTR);
    m_atom = reply->atom;
    free(reply);
    m_retrieved = true;
}

} // namespace Xcb
