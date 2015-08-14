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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
 *
 *     You may use this file under the terms of the BSD license as follows:
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions are
 *     met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in
 *         the documentation and/or other materials provided with the
 *         distribution.
 *       * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 *         of its contributors may be used to endorse or promote products derived
 *         from this software without specific prior written permission.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ***************************************************************************/

#ifndef GREENISLAND_INPUTPANEL_H
#define GREENISLAND_INPUTPANEL_H

#include <QtCore/QObject>
#include <QtCore/QRect>

#include <greenisland/greenisland_export.h>

namespace GreenIsland {

class InputPanelPrivate;
class Surface;

class WlInputMethod;
class WlTextInput;

class GREENISLAND_EXPORT InputPanel : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(InputPanel)
    Q_PROPERTY(Surface* focus READ focus NOTIFY focusChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)
    Q_PROPERTY(QRect cursorRectangle READ cursorRectangle NOTIFY cursorRectangleChanged)
public:
    InputPanel(QObject *parent = 0);

    Surface *focus() const;
    bool visible() const;
    QRect cursorRectangle() const;

Q_SIGNALS:
    void focusChanged();
    void visibleChanged();
    void cursorRectangleChanged();

private:
    friend class WlInputMethod;
    friend class WlTextInput;
};

} // namespace GreenIsland

#endif // GREENISLAND_INPUTPANEL_H
