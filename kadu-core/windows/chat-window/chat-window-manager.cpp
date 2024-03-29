/*
 * %kadu copyright begin%
 * Copyright 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "chat-window-manager.h"
#include "moc_chat-window-manager.cpp"

#include "chat/chat.h"
#include "widgets/chat-widget/chat-widget-manager.h"
#include "windows/chat-window/chat-window-repository.h"
#include "windows/chat-window/chat-window-storage.h"
#include "windows/chat-window/chat-window.h"

ChatWindowManager::ChatWindowManager(QObject *parent) : QObject(parent)
{
}

ChatWindowManager::~ChatWindowManager()
{
}
void ChatWindowManager::setChatWidgetManager(ChatWidgetManager *chatWidgetManager)
{
    m_chatWidgetManager = chatWidgetManager;
}

void ChatWindowManager::setChatWindowRepository(ChatWindowRepository *chatWindowRepository)
{
    m_chatWindowRepository = chatWindowRepository;
}

void ChatWindowManager::setChatWindowStorage(ChatWindowStorage *chatWindowStorage)
{
    m_chatWindowStorage = chatWindowStorage;
}

void ChatWindowManager::openStoredChatWindows()
{
    if (!m_chatWindowStorage || !m_chatWindowRepository)
        return;

    auto chats = m_chatWindowStorage.data()->loadChats();
    for (auto const &chat : chats)
        m_chatWidgetManager.data()->openChat(chat, OpenChatActivation::DoNotActivate);
}

void ChatWindowManager::storeOpenedChatWindows()
{
    if (!m_chatWindowRepository)
        return;

    auto chats = QVector<Chat>{};
    std::transform(
        begin(m_chatWindowRepository.data()), end(m_chatWindowRepository.data()), std::back_inserter(chats),
        [](ChatWindow *window) { return window->chat(); });

    if (m_chatWindowStorage)
        m_chatWindowStorage.data()->storeChats(chats);
}
