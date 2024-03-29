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

#include "chat/chat-details-contact.h"
#include "chat/chat-manager.h"
#include "chat/chat-storage.h"
#include "chat/chat.h"
#include "contacts/contact-set.h"
#include "core/injected-factory.h"
#include "icons/kadu-icon.h"

#include "chat-type-contact.h"
#include "moc_chat-type-contact.cpp"

Chat ChatTypeContact::findChat(
    ChatManager *chatManager, ChatStorage *chatStorage, const Contact &contact, NotFoundAction notFoundAction)
{
    Account account = contact.contactAccount();
    if (!account)
        return Chat::null;

    if (contact.id() == account.id())
        return Chat::null;

    for (auto const &chat : chatManager->items())
        if (chat.type() == QStringLiteral("Contact") || chat.type() == QStringLiteral("Simple"))
            if (chat.contacts().toContact() == contact)
            {
                // when contacts changed their accounts we need to change account of chat too
                chat.setChatAccount(account);
                return chat;
            }

    if (ActionReturnNull == notFoundAction)
        return Chat::null;

    auto chat = chatStorage->create("Contact");
    chat.setChatAccount(account);

    ChatDetailsContact *chatDetailsContact = dynamic_cast<ChatDetailsContact *>(chat.details());
    if (chatDetailsContact)
    {
        chatDetailsContact->setState(StorableObject::StateNew);
        chatDetailsContact->setContact(contact);
    }

    if (ActionCreateAndAdd == notFoundAction)
        chatManager->addItem(chat);

    return chat;
}

ChatTypeContact::ChatTypeContact(QObject *parent) : ChatType(parent)
{
    Aliases.append("Contact");
    Aliases.append("Simple");
}

ChatTypeContact::~ChatTypeContact()
{
}

void ChatTypeContact::setInjectedFactory(InjectedFactory *injectedFactory)
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
 * Internal name for ChatTypeContact is 'Contact'.
 */
QString ChatTypeContact::name() const
{
    return "Contact";
}

/**
 * @short Internal aliases of chat type.
 * @return internal aliases of chat type
 *
 * Chat type internal aliases, used to imporitng old configurations.
 *
 * Internal aliases for ChatTypeContact are 'Contact' and 'Simple'.
 */
QStringList ChatTypeContact::aliases() const
{
    return Aliases;
}

/**
 * @short Icon of chat type.
 * @return icon of chat type
 *
 * Chat type icon. Icon is used in history window and as icon of chat windows.
 *
 * Icon for ChatTypeContact is 'internet-group-chat'.
 */
KaduIcon ChatTypeContact::icon() const
{
    return KaduIcon("internet-group-chat");
}

/**
 * @short Window role for this chat type.
 * @return window role for this chat type.
 *
 * Kadu window role for this chat type.
 * For the contact chat the role is "kadu-chat-contact".
 */
QString ChatTypeContact::windowRole() const
{
    return QStringLiteral("kadu-chat-contact");
}

/**
 * @short Creates new ChatDetailsContact object for given chat type.
 * @return new ChatDetailsContact object for given chat type
 *
 * Creates new @link ChatDetailsContact @endlink object for
 * given @link Chat @endlink (@link ChatShared @endlink).
 */
ChatDetails *ChatTypeContact::createChatDetails(ChatShared *chatData) const
{
    return m_injectedFactory->makeInjected<ChatDetailsContact>(chatData);
}

ChatEditWidget *ChatTypeContact::createEditWidget(const Chat &chat, QWidget *parent) const
{
    Q_UNUSED(chat);
    Q_UNUSED(parent);

    return 0;
}
