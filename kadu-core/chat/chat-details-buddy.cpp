/*
 * %kadu copyright begin%
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "chat-details-buddy.h"
#include "moc_chat-details-buddy.cpp"

#include "chat/chat.h"
#include "chat/type/chat-type-manager.h"
#include "contacts/contact-set.h"

/**
 * @short Creates empty ChatDetailsBuddy object.
 * @param chatData Chat object that will be decribed by this object
 *
 * Creates empty ChatDetailsBuddy object assigned to chatData object.
 */
ChatDetailsBuddy::ChatDetailsBuddy(ChatShared *chatData) : ChatDetails(chatData)
{
}

ChatDetailsBuddy::~ChatDetailsBuddy()
{
}

void ChatDetailsBuddy::setChatTypeManager(ChatTypeManager *chatTypeManager)
{
    m_chatTypeManager = chatTypeManager;
}

/**
 * @short Return false, this class should never be stored.
 * @return false, this class should never be stored
 *
 * Return false, this class should never be stored.
 */
bool ChatDetailsBuddy::shouldStore()
{
    return false;
}

/**
 * @short Returns type of this chat - 'Buddy'.
 * @return 'Buddy' ChatType object
 *
 * Returns type of this chat - 'Buddy'.
 */
ChatType *ChatDetailsBuddy::type() const
{
    return m_chatTypeManager->chatType("Buddy");
}

/**
 * @short Returns set of contracts of first chat in this aggregate chat or empty set.
 * @return set of contracts of first chat in this aggregate chat or empty set
 *
 * Returns set of contracts of first chat in this aggregate chat or empty set
 * no chats are aggreagated.
 */
ContactSet ChatDetailsBuddy::contacts() const
{
    return m_chats.isEmpty() ? ContactSet() : m_chats.at(0).contacts();
}

/**
 * @short Returns name of first chat in this aggregate chat or empty string.
 * @return name of first chat in this aggregate chat or empty string
 *
 * Returns name of first chat in this aggregate chat or empty string if
 * no chats are aggreagated.
 */
QString ChatDetailsBuddy::name() const
{
    return m_chats.isEmpty() ? m_buddy.isEmpty() ? QString() : m_buddy.display() : m_chats.at(0).name();
}

bool ChatDetailsBuddy::isConnected() const
{
    return false;
}

void ChatDetailsBuddy::setBuddy(const Buddy &buddy)
{
    m_buddy = buddy;
}

Buddy ChatDetailsBuddy::buddy() const
{
    return m_buddy;
}

/**
 * @short Assigns aggregate chats to this chat.
 * @param chats chats to be assigned
 *
 * Assigns aggregate chats to this chat.
 */
void ChatDetailsBuddy::setChats(const QVector<Chat> &chats)
{
    m_chats.clear();
    for (auto &&chat : chats)
        addChat(chat);
}

/**
 * @short Returns assigned aggregate chats to this chat.
 * @return assigned aggregate chats to this chat
 *
 * Returns assigned aggregate chats to this chat.
 */
const QVector<Chat> &ChatDetailsBuddy::chats() const
{
    return m_chats;
}

void ChatDetailsBuddy::addChat(const Chat &chat)
{
    if (chat && !m_chats.contains(chat))
        m_chats.append(chat);
}

void ChatDetailsBuddy::removeChat(const Chat &chat)
{
    int indexOf = m_chats.indexOf(chat);
    if (indexOf >= 0)
        m_chats.remove(indexOf);
}
