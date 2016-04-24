/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2016 Pier Luigi Fiorini
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

#ifndef GREENISLANDCLIENT_OUTPUTCONFIGURATION_H
#define GREENISLANDCLIENT_OUTPUTCONFIGURATION_H

#include <QtCore/QObject>

#include <GreenIsland/Client/Output>

namespace GreenIsland {

namespace Client {

class OutputConfigurationPrivate;
class OutputManagement;

class GREENISLANDCLIENT_EXPORT OutputConfiguration : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(OutputConfiguration)
public:
    void setEnabled(Output *output, bool value);
    void setPrimary(Output *output, bool value);
    void setModeId(Output *output, int modeId);
    void setTransform(Output *output, Output::Transform transform);
    void setPosition(Output *output, const QPoint &position);
    void setScaleFactor(Output *output, qint32 scaleFactor);

    void apply();

private:
    explicit OutputConfiguration(QObject *parent = Q_NULLPTR);

    friend class OutputManagement;
};

} // namespace Client

} // namespace GreenIsland

#endif // GREENISLANDCLIENT_OUTPUTCONFIGURATION_H
