/*
 * %kadu copyright begin%
 * Copyright 2012 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "chats-buddies-splitter.h"
#include "moc_chats-buddies-splitter.cpp"

#include "buddies/buddy-manager.h"
#include "chat/buddy-chat-manager.h"
#include "chat/chat-details-buddy.h"
#include "chat/chat.h"
#include "chat/type/chat-type-manager.h"
#include "contacts/contact-set.h"
#include "talkable/talkable-converter.h"
#include "talkable/talkable.h"

#include <QtCore/QSet>

ChatsBuddiesSplitter::ChatsBuddiesSplitter(QVector<Talkable> talkables, QObject *parent)
        : QObject{parent}, m_talkables{talkables}
{
}

ChatsBuddiesSplitter::~ChatsBuddiesSplitter()
{
}

void ChatsBuddiesSplitter::setBuddyChatManager(BuddyChatManager *buddyChatManager)
{
    m_buddyChatManager = buddyChatManager;
}

void ChatsBuddiesSplitter::setBuddyManager(BuddyManager *buddyManager)
{
    m_buddyManager = buddyManager;
}

void ChatsBuddiesSplitter::setChatTypeManager(ChatTypeManager *chatTypeManager)
{
    m_chatTypeManager = chatTypeManager;
}

void ChatsBuddiesSplitter::setTalkableConverter(TalkableConverter *talkableConverter)
{
    m_talkableConverter = talkableConverter;
}

void ChatsBuddiesSplitter::init()
{
    // we ignore contacts
    for (auto const &talkable : m_talkables)
        if (talkable.isValidChat())
            processChat(m_talkableConverter->toChat(talkable));
        else if (talkable.isValidBuddy())
            Buddies.insert(m_talkableConverter->toBuddy(talkable));
}

void ChatsBuddiesSplitter::processChat(const Chat &chat)
{
    if (UsedChats.contains(chat))
        return;

    auto buddyChat = m_buddyChatManager->buddyChat(chat);
    if (!buddyChat)
    {
        UsedChats.insert(chat);
        assignChat(chat);
        return;
    }

    ChatDetailsBuddy *details = qobject_cast<ChatDetailsBuddy *>(buddyChat.details());
    Q_ASSERT(details);

    for (auto const &usedChat : details->chats())
        UsedChats.insert(usedChat);
    assignChat(buddyChat);
}

void ChatsBuddiesSplitter::assignChat(const Chat &chat)
{
    ChatType *chatType = m_chatTypeManager->chatType(chat.type());
    if (chatType && (chatType->name() == "Contact" || chatType->name() == "Buddy"))
        Buddies.insert(m_buddyManager->byContact(*chat.contacts().begin(), ActionCreateAndAdd));
    else
        Chats.insert(chat);
}

QSet<Chat> ChatsBuddiesSplitter::chats() const
{
    return Chats;
}

QSet<Buddy> ChatsBuddiesSplitter::buddies() const
{
    return Buddies;
}
