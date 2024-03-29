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

#include "chat-type-contact-set.h"
#include "moc_chat-type-contact-set.cpp"

#include "chat/chat-details-contact-set.h"
#include "chat/chat-manager.h"
#include "chat/chat-storage.h"
#include "core/injected-factory.h"
#include "icons/kadu-icon.h"

Chat ChatTypeContactSet::findChat(
    ChatManager *chatManager, ChatStorage *chatStorage, const ContactSet &contacts, NotFoundAction notFoundAction)
{
    if (contacts.count() < 2)
        return Chat::null;

    Account account = (*contacts.constBegin()).contactAccount();
    if (account.isNull())
        return Chat::null;

    for (auto const &contact : contacts)
        if (account != contact.contactAccount())
            return Chat::null;

    // TODO #1694
    // for some users that have self on user list
    // this should not be possible, and prevented on other level (like in ContactManager)
    for (auto const &contact : contacts)
        if (contact.id() == account.id())
            return Chat::null;

    for (auto const &chat : chatManager->items())
        if (chat.type() == QStringLiteral("ContactSet") || chat.type() == QStringLiteral("Conference"))
            if (chat.contacts() == contacts)
            {
                // when contacts changed their accounts we need to change account of chat too
                chat.setChatAccount(account);
                return chat;
            }

    if (ActionReturnNull == notFoundAction)
        return Chat::null;

    auto chat = chatStorage->create("ContactSet");
    chat.setChatAccount(account);

    Contact contact = contacts.toContact();

    // only gadu-gadu support contact-sets
    // TODO: this should be done better
    if (chat.chatAccount().protocolName() != "gadu")
        return Chat::null;

    ChatDetailsContactSet *chatDetailsContactSet = dynamic_cast<ChatDetailsContactSet *>(chat.details());
    if (chatDetailsContactSet)
    {
        chatDetailsContactSet->setState(StorableObject::StateNew);
        chatDetailsContactSet->setContacts(contacts);
    }

    if (ActionCreateAndAdd == notFoundAction)
        chatManager->addItem(chat);

    return chat;
}

ChatTypeContactSet::ChatTypeContactSet(QObject *parent) : ChatType(parent)
{
    m_aliases.append("ContactSet");
    m_aliases.append("Conference");
}

ChatTypeContactSet::~ChatTypeContactSet()
{
}

void ChatTypeContactSet::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

/**
 * @short Internal name of chat type.
 * @return internal name of chat type
 *
 * Chat type internal name. Internal name is used in @link ChatTypeManager @endlink
 * and also it is stored with @link Chat @endlink data.
 *
 * Internal name for ChatTypeContactSet is 'ContactSet'.
 */
QString ChatTypeContactSet::name() const
{
    return "ContactSet";
}

/**
 * @short Internal aliases of chat type.
 * @return internal aliases of chat type
 *
 * Chat type internal aliases, used to imporitng old configurations.
 *
 * Internal aliases for ChatTypeContactSet are 'ContactSet' and 'Conference'.
 */
QStringList ChatTypeContactSet::aliases() const
{
    return m_aliases;
}

/**
 * @short Icon of chat type.
 * @return icon of chat type
 *
 * Chat type icon. Icon is used in history window and as icon of chat windows.
 *
 * Icon for ChatTypeContactSet is 'kadu_icons/conference'.
 */
KaduIcon ChatTypeContactSet::icon() const
{
    return KaduIcon("kadu_icons/conference");
}

/**
 * @short Window role for this chat type.
 * @return window role for this chat type.
 *
 * Kadu window role for this chat type.
 * For ContactSet the role is "kadu-chat-contact-set".
 */
QString ChatTypeContactSet::windowRole() const
{
    return "kadu-chat-contact-set";
}

/**
 * @short Creates new ChatDetailsContactSet object for given chat type.
 * @return new ChatDetailsContactSet object for given chat type
 *
 * Creates new @link ChatDetailsContactSet @endlink object for
 * given @link Chat @endlink (@link ChatShared @endlink).
 */
ChatDetails *ChatTypeContactSet::createChatDetails(ChatShared *chatData) const
{
    return m_injectedFactory->makeInjected<ChatDetailsContactSet>(chatData);
}

ChatEditWidget *ChatTypeContactSet::createEditWidget(const Chat &chat, QWidget *parent) const
{
    Q_UNUSED(chat);
    Q_UNUSED(parent);

    return 0;
}
