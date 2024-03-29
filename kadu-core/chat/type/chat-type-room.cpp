/*
 * %kadu copyright begin%
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "chat/chat-details-room.h"
#include "chat/chat-manager.h"
#include "chat/chat-storage.h"
#include "core/injected-factory.h"
#include "icons/kadu-icon.h"
#include "widgets/chat-room-edit-widget.h"

#include "chat-type-room.h"
#include "moc_chat-type-room.cpp"

Chat ChatTypeRoom::findChat(
    ChatManager *chatManager, ChatStorage *chatStorage, const Account &account, const QString &room,
    NotFoundAction notFoundAction)
{
    if (!account)
        return Chat::null;

    for (auto const &chat : chatManager->items())
    {
        if (chat.type() != "Room")
            continue;
        if (chat.chatAccount() != account)
            continue;

        ChatDetailsRoom *details = qobject_cast<ChatDetailsRoom *>(chat.details());
        if (!details)
            continue;

        if (details->room() == room)
            return chat;
    }

    if (ActionReturnNull == notFoundAction)
        return Chat::null;

    auto chat = chatStorage->create("Room");
    chat.setChatAccount(account);

    ChatDetailsRoom *details = qobject_cast<ChatDetailsRoom *>(chat.details());
    Q_ASSERT(details);

    details->setRoom(room);

    if (ActionCreateAndAdd == notFoundAction)
        chatManager->addItem(chat);

    return chat;
}

ChatTypeRoom::ChatTypeRoom(QObject *parent) : ChatType(parent)
{
}

ChatTypeRoom::~ChatTypeRoom()
{
}

void ChatTypeRoom::setInjectedFactory(InjectedFactory *injectedFactory)
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
 * Internal name for ChatTypeRoom is 'Room'.
 */
QString ChatTypeRoom::name() const
{
    return "Room";
}

/**
 * @short Internal aliases of chat type.
 * @return internal aliases of chat type
 *
 * Chat type internal aliases, used to imporitng old configurations.
 *
 * Internal aliase for ChatTypeRoom is 'Room'.
 */
QStringList ChatTypeRoom::aliases() const
{
    return QStringList() << name();
}

/**
 * @short Icon of chat type.
 * @return icon of chat type
 *
 * Chat type icon. Icon is used in history window and as icon of chat windows.
 *
 * Icon for ChatTypeRoom is 'kadu_icons/conference'.
 */
KaduIcon ChatTypeRoom::icon() const
{
    return KaduIcon("kadu_icons/conference");
}

/**
 * @short Window role for this chat type.
 * @return window role for this chat type.
 *
 * Kadu window role for this chat type.
 * For ContactSet the role is "kadu-chat-room".
 */
QString ChatTypeRoom::windowRole() const
{
    return "kadu-chat-room";
}

/**
 * @short Creates new ChatDetailsRoom object for given chat type.
 * @return new ChatDetailsRoom object for given chat type
 *
 * Creates new @link ChatDetailsRoom @endlink object for
 * given @link Chat @endlink (@link ChatShared @endlink).
 */
ChatDetails *ChatTypeRoom::createChatDetails(ChatShared *chatData) const
{
    return m_injectedFactory->makeInjected<ChatDetailsRoom>(chatData);
}

ChatEditWidget *ChatTypeRoom::createEditWidget(const Chat &chat, QWidget *parent) const
{
    return m_injectedFactory->makeInjected<ChatRoomEditWidget>(chat, parent);
}
