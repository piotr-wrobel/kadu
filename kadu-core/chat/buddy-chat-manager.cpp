/*
 * %kadu copyright begin%
 * Copyright 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "buddy-chat-manager.h"
#include "moc_buddy-chat-manager.cpp"

#include "accounts/account-manager.h"
#include "buddies/buddy-manager.h"
#include "chat/chat-details-buddy.h"
#include "chat/chat-details-contact.h"
#include "chat/chat-manager.h"
#include "chat/chat-storage.h"
#include "chat/type/chat-type-contact.h"
#include "contacts/contact-set.h"
#include "protocols/protocol.h"

BuddyChatManager::BuddyChatManager(QObject *parent) : QObject{parent}
{
}

BuddyChatManager::~BuddyChatManager()
{
}

void BuddyChatManager::setBuddyManager(BuddyManager *buddyManager)
{
    m_buddyManager = buddyManager;
}

void BuddyChatManager::setChatManager(ChatManager *chatManager)
{
    m_chatManager = chatManager;
}

void BuddyChatManager::setChatStorage(ChatStorage *chatStorage)
{
    m_chatStorage = chatStorage;
}

void BuddyChatManager::init()
{
    connect(m_buddyManager, SIGNAL(buddyContactAdded(Buddy, Contact)), this, SLOT(buddyContactAdded(Buddy, Contact)));
    connect(
        m_buddyManager, SIGNAL(buddyContactRemoved(Buddy, Contact)), this, SLOT(buddyContactRemoved(Buddy, Contact)));

    connect(m_chatManager, SIGNAL(chatAdded(Chat)), this, SLOT(chatAdded(Chat)));
    connect(m_chatManager, SIGNAL(chatRemoved(Chat)), this, SLOT(chatRemoved(Chat)));

    for (auto const &chat : m_chatManager->items())
        chatAdded(chat);
}

void BuddyChatManager::done()
{
    for (auto const &chat : m_chatManager->items())
        chatRemoved(chat);
}

Chat BuddyChatManager::createAndInsertBuddyChat(const Buddy &buddy)
{
    auto result = m_chatStorage->create("Buddy");
    ChatDetailsBuddy *buddyDetails = qobject_cast<ChatDetailsBuddy *>(result.details());
    Q_ASSERT(buddyDetails);

    QVector<Chat> chats;
    for (auto const &contact : buddy.contacts())
    {
        auto contactChat = ChatTypeContact::findChat(m_chatManager, m_chatStorage, contact, ActionCreateAndAdd);
        if (contactChat)
            chats.append(contactChat);
    }

    buddyDetails->setBuddy(buddy);
    buddyDetails->setChats(chats);

    BuddyChats.insert(buddy, result);

    return result;
}

void BuddyChatManager::buddyContactAdded(const Buddy &buddy, const Contact &contact)
{
    Chat chat = buddyChat(buddy);
    ChatDetailsBuddy *buddyDetails = qobject_cast<ChatDetailsBuddy *>(chat.details());
    Q_ASSERT(buddyDetails);

    buddyDetails->addChat(ChatTypeContact::findChat(m_chatManager, m_chatStorage, contact, ActionReturnNull));
}

void BuddyChatManager::buddyContactRemoved(const Buddy &buddy, const Contact &contact)
{
    Chat chat = BuddyChats.value(buddy);
    if (!chat)
        return;

    ChatDetailsBuddy *buddyDetails = qobject_cast<ChatDetailsBuddy *>(chat.details());
    Q_ASSERT(buddyDetails);

    buddyDetails->removeChat(ChatTypeContact::findChat(m_chatManager, m_chatStorage, contact, ActionReturnNull));
}

void BuddyChatManager::chatAdded(const Chat &addedChat)
{
    ChatDetailsContact *contactDetails = qobject_cast<ChatDetailsContact *>(addedChat.details());
    if (!contactDetails || !contactDetails->contact().ownerBuddy())
        return;

    Chat chat = buddyChat(contactDetails->contact().ownerBuddy());
    ChatDetailsBuddy *chatDetails = qobject_cast<ChatDetailsBuddy *>(chat.details());
    Q_ASSERT(chatDetails);

    chatDetails->addChat(addedChat);
}

void BuddyChatManager::chatRemoved(const Chat &removedChat)
{
    ChatDetailsContact *contactDetails = qobject_cast<ChatDetailsContact *>(removedChat.details());
    if (!contactDetails || !contactDetails->contact().ownerBuddy())
        return;

    Chat chat = BuddyChats.value(contactDetails->contact().ownerBuddy());
    if (!chat)
        return;

    ChatDetailsBuddy *chatDetails = qobject_cast<ChatDetailsBuddy *>(chat.details());
    Q_ASSERT(chatDetails);

    chatDetails->removeChat(removedChat);
}

Chat BuddyChatManager::buddyChat(const Chat &chat)
{
    ChatDetailsBuddy *buddyDetails = qobject_cast<ChatDetailsBuddy *>(chat.details());
    if (buddyDetails)
        return chat;

    ChatDetailsContact *contactDetails = qobject_cast<ChatDetailsContact *>(chat.details());
    if (!contactDetails)
        return Chat::null;

    return buddyChat(contactDetails->contact().ownerBuddy());
}

Chat BuddyChatManager::buddyChat(const Buddy &buddy)
{
    if (!buddy)
        return Chat::null;

    if (BuddyChats.contains(buddy))
        return BuddyChats.value(buddy);
    else
        return createAndInsertBuddyChat(buddy);
}
