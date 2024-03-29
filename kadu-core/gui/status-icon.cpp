/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QTimer>

#include "status/status-container.h"

#include "status-icon.h"
#include "moc_status-icon.cpp"

StatusIcon::StatusIcon(StatusContainer *statusContainer, QObject *parent)
        : QObject(parent), MyStatusContainer(statusContainer), BlinkTimer(0), BlinkOffline(true)
{
}

StatusIcon::~StatusIcon()
{
}

void StatusIcon::init()
{
    statusUpdated();
    connect(MyStatusContainer, SIGNAL(statusUpdated(StatusContainer *)), this, SLOT(statusUpdated(StatusContainer *)));
}

void StatusIcon::enableBlink()
{
    if (BlinkTimer)
        return;

    BlinkTimer = new QTimer(this);
    connect(BlinkTimer, SIGNAL(timeout()), this, SLOT(blink()));
    BlinkTimer->start(500);
}

void StatusIcon::disableBlink()
{
    if (!BlinkTimer)
        return;

    delete BlinkTimer;
    BlinkTimer = 0;

    setIcon(MyStatusContainer->statusIcon());
}

void StatusIcon::blink()
{
    if (!MyStatusContainer->isStatusSettingInProgress())
    {
        disableBlink();
        return;
    }

    BlinkOffline = !BlinkOffline;

    if (BlinkOffline)
        setIcon(MyStatusContainer->statusIcon(Status{StatusType::Offline}));
    else
        setIcon(MyStatusContainer->statusIcon());
}

void StatusIcon::updateStatus()
{
    if (!MyStatusContainer->isStatusSettingInProgress())
        setIcon(MyStatusContainer->statusIcon());
    else
        enableBlink();
}

void StatusIcon::statusUpdated(StatusContainer *container)
{
    Q_UNUSED(container)

    updateStatus();
}

void StatusIcon::configurationUpdated()
{
    updateStatus();
}

void StatusIcon::setIcon(const KaduIcon &icon)
{
    Icon = icon;
    emit iconUpdated(Icon);
}
