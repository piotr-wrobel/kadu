/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "serial-avatar-downloader.h"
#include "serial-avatar-downloader.moc"

#include "avatars/aggregated-contact-avatar-service.h"
#include "avatars/contact-avatar-global-id.h"

#include <QtCore/QTimer>

SerialAvatarDownloader::SerialAvatarDownloader(QObject *parent) : QObject{parent}
{
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, &SerialAvatarDownloader::downloadNextAvatar);
}

SerialAvatarDownloader::~SerialAvatarDownloader()
{
}

void SerialAvatarDownloader::setAggregatedContactAvatarService(
    AggregatedContactAvatarService *aggregatedContactAvatarService)
{
    m_aggregatedContactAvatarService = aggregatedContactAvatarService;
}

void SerialAvatarDownloader::downloadNextAvatar()
{
    if (m_avatarsToDownload.empty())
    {
        m_timer.stop();
        return;
    }

    m_aggregatedContactAvatarService->download(m_avatarsToDownload.back());
    m_avatarsToDownload.pop_back();
}

void SerialAvatarDownloader::downloadAvatar(const ContactAvatarGlobalId &id)
{
    m_avatarsToDownload.push_back(id);
    if (!m_timer.isActive())
        m_timer.start();
}
