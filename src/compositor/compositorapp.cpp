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

#include "compositorapp.h"

CompositorApp::CompositorApp(int &argc, char **argv)
    : QGuiApplication(argc, argv)
    , m_fakeScreenCount(0)
    , m_fakeScreenSize(QSize(1024, 768))
{
}

int CompositorApp::fakeScreenCount() const
{
    return m_fakeScreenCount;
}

void CompositorApp::setFakeScreenCount(int count)
{
    m_fakeScreenCount = count;
}

QSize CompositorApp::fakeScreenSize() const
{
    return m_fakeScreenSize;
}

void CompositorApp::setFakeScreenSize(const QSize &size)
{
    m_fakeScreenSize = size;
}

#include "moc_compositorapp.cpp"
