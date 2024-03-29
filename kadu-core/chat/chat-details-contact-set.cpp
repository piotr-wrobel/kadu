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

#include "chat-details-contact-set.h"
#include "moc_chat-details-contact-set.cpp"

#include "buddies/buddy-manager.h"
#include "chat/chat.h"
#include "chat/type/chat-type-manager.h"
#include "contacts/contact-manager.h"
#include "contacts/contact-set-configuration-helper.h"
#include "protocols/protocol.h"

/**
 * @short Creates empty ChatDetailsContactSet object.
 * @param chatData Chat object that will be decribed by this object
 *
 * Creates empty ChatDetailsContactSet object assigned to chatData object.
 */
ChatDetailsContactSet::ChatDetailsContactSet(ChatShared *chatData) : ChatDetails(chatData)
{
    Protocol *protocol = mainData()->chatAccount().protocolHandler();

    if (protocol)
    {
        connect(protocol, SIGNAL(connected(Account)), this, SIGNAL(connected()));
        connect(protocol, SIGNAL(disconnected(Account)), this, SIGNAL(disconnected()));
    }
}

ChatDetailsContactSet::~ChatDetailsContactSet()
{
}

void ChatDetailsContactSet::setChatTypeManager(ChatTypeManager *chatTypeManager)
{
    m_chatTypeManager = chatTypeManager;
}

void ChatDetailsContactSet::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

/**
 * @short Loads ChatDetailsContactSet object from storage.
 *
 * Loads ChatDetailsContactSet object from the same storage assigned Chat object is
 * using. This loads set of contacts from 'Contacts' subnode.
 */
void ChatDetailsContactSet::load()
{
    if (!isValidStorage())
        return;

    ChatDetails::load();

    m_contacts = ContactSetConfigurationHelper::loadFromConfiguration(m_contactManager, this, "Contacts");
}

/**
 * @short Stores ChatDetailsContactSet object to storage.
 *
 * Stores ChatDetailsContactSet object to the same storage assigned Chat object is
 * using. This stores set of contacts into 'm_contacts' subnode.
 */
void ChatDetailsContactSet::store()
{
    if (!isValidStorage())
        return;

    ensureLoaded();

    ContactSetConfigurationHelper::saveToConfiguration(this, "Contacts", m_contacts);
}

/**
 * @short Returns true if assigned set of contact is valid.
 * @return true if assigned set of contact is valid
 *
 * Returns true if assigned set of contacts is not empty. No empty chats (without contacts)
 * will be stored thanks to this method.
 */
bool ChatDetailsContactSet::shouldStore()
{
    ensureLoaded();

    return StorableObject::shouldStore() && !m_contacts.isEmpty();
}

/**
 * @short Returns type of this chat - 'ContactSet'.
 * @return 'ContactSet' ChatType object
 *
 * Returns type of this chat - 'ContactSet'.
 */
ChatType *ChatDetailsContactSet::type() const
{
    return m_chatTypeManager->chatType("ContactSet");
}

/**
 * @short Returns name of this chat.
 * @return name of this chat
 *
 * Returns name of this chat (which is display names of assigend contacts buddies
 * joined by commas).
 */
QString ChatDetailsContactSet::name() const
{
    QStringList displays;
    for (auto const &contact : m_contacts)
        displays.append(contact.display(true));

    displays.sort();
    return displays.join(", ");
}

bool ChatDetailsContactSet::isConnected() const
{
    return mainData()->chatAccount().protocolHandler() && mainData()->chatAccount().protocolHandler()->isConnected();
}

/**
 * @short Assigns contact set to this chat.
 * @param contacts contact set to assign
 *
 * Assigns contact set to this chat.
 */
void ChatDetailsContactSet::setContacts(const ContactSet &contacts)
{
    ensureLoaded();

    m_contacts = contacts;
}
