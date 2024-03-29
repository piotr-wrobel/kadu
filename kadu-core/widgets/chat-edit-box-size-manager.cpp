/*
 * %kadu copyright begin%
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

#include "chat-edit-box-size-manager.h"
#include "moc_chat-edit-box-size-manager.cpp"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "core/core.h"

ChatEditBoxSizeManager::ChatEditBoxSizeManager(QObject *parent) : QObject{parent}, CommonHeight{}
{
}

ChatEditBoxSizeManager::~ChatEditBoxSizeManager()
{
}

void ChatEditBoxSizeManager::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void ChatEditBoxSizeManager::init()
{
    configurationUpdated();
}

void ChatEditBoxSizeManager::configurationUpdated()
{
    setCommonHeight(m_configuration->deprecatedApi()->readNumEntry("Chat", "ChatEditBoxHeight", 0));
}

void ChatEditBoxSizeManager::setCommonHeight(int height)
{
    if (height != CommonHeight)
    {
        CommonHeight = height;
        m_configuration->deprecatedApi()->writeEntry("Chat", "ChatEditBoxHeight", CommonHeight);
        emit commonHeightChanged(CommonHeight);
    }
}

bool ChatEditBoxSizeManager::initialized()
{
    return 0 != CommonHeight;
}
