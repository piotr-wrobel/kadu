/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "sound-notifier.h"
#include "moc_sound-notifier.cpp"

#include "gui/sound-configuration-ui-handler.h"
#include "sound-manager.h"

#include "chat/chat.h"
#include "contacts/contact-set.h"
#include "notification/notification-configuration.h"
#include "notification/notification.h"
#include "notification/notifier-repository.h"

#include <QtCore/QFileInfo>

SoundNotifier::SoundNotifier(QObject *parent)
        : QObject{parent}, Notifier{"Sound", QT_TRANSLATE_NOOP("@default", "Play a sound"),
                                    KaduIcon{"audio-volume-high"}}
{
}

SoundNotifier::~SoundNotifier()
{
}

void SoundNotifier::setNotificationConfiguration(NotificationConfiguration *notificationConfiguration)
{
    m_notificationConfiguration = notificationConfiguration;
}

void SoundNotifier::setNotifierRepository(NotifierRepository *notifierRepository)
{
    m_notifierRepository = notifierRepository;
}

void SoundNotifier::setSoundConfigurationUiHandler(SoundConfigurationUiHandler *soundConfigurationUiHandler)
{
    m_soundConfigurationUiHandler = soundConfigurationUiHandler;
}

void SoundNotifier::setSoundManager(SoundManager *soundManager)
{
    m_soundManager = soundManager;
}

void SoundNotifier::notify(const Notification &notification)
{
    auto chat = notification.data["chat"].value<Chat>();
    if (chat && chat.property("sound:use_custom_sound", false).toBool())
    {
        // we need abstraction for that
        auto customSound = chat.property("sound:custom_sound", QString{}).toString();
        auto fileInfo = QFileInfo{customSound};
        if (fileInfo.exists())
        {
            m_soundManager->playFile(customSound);
            return;
        }
    }

    if (!chat.contacts().isEmpty())
    {
        auto buddy = chat.contacts().begin()->ownerBuddy();
        if (buddy && buddy.property("sound:use_custom_sound", false).toBool())
        {
            auto customSound = buddy.property("sound:custom_sound", QString{}).toString();
            auto fileInfo = QFileInfo{customSound};
            if (fileInfo.exists())
            {
                m_soundManager->playFile(customSound);
                return;
            }
        }
    }

    auto key = m_notificationConfiguration->notifyConfigurationKey(notification.type);
    m_soundManager->playSoundByName(key);
}

NotifierConfigurationWidget *SoundNotifier::createConfigurationWidget(QWidget *parent)
{
    return m_soundConfigurationUiHandler ? m_soundConfigurationUiHandler->createConfigurationWidget(parent) : nullptr;
}
