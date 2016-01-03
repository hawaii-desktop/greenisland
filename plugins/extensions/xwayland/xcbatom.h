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

#ifndef XCB_ATOM_H
#define XCB_ATOM_H

#include <QtCore/QByteArray>

#include "xwaylandmanager.h"

namespace Xcb {

class Atom
{
    Q_DISABLE_COPY(Atom)
public:
    explicit Atom(const QByteArray &name, bool onlyIfExists = false);
    Atom();
    ~Atom();

    inline const QByteArray &name() const {
        return m_name;
    }

    bool isValid();

    operator xcb_atom_t() const;

    static QString nameFromAtom(xcb_atom_t atom);

private:
    QByteArray m_name;
    bool m_retrieved;
    xcb_intern_atom_cookie_t m_cookie;
    xcb_atom_t m_atom;

    void getReply();
};

} // namespace Xcb

#endif // XCB_ATOM_H
