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

#include "chat-type-buddy.h"
#include "moc_chat-type-buddy.cpp"

#include "chat/chat-details-buddy.h"
#include "chat/chat.h"
#include "core/injected-factory.h"
#include "icons/kadu-icon.h"

ChatTypeBuddy::ChatTypeBuddy(QObject *parent) : ChatType(parent)
{
    m_aliases.append(QStringLiteral("Buddy"));
    m_aliases.append(QStringLiteral("Aggregate"));
}

ChatTypeBuddy::~ChatTypeBuddy()
{
}

void ChatTypeBuddy::setInjectedFactory(InjectedFactory *injectedFactory)
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
 * Internal name for ChatTypeBuddy is 'Buddy'.
 */
QString ChatTypeBuddy::name() const
{
    return "Buddy";
}

/**
 * @short Internal aliases of chat type.
 * @return internal aliases of chat type
 *
 * Chat type internal aliases, used to imporitng old configurations.
 *
 * Internal aliases for ChatTypeBuddy are 'Buddy' and 'Aggregate'.
 */
QStringList ChatTypeBuddy::aliases() const
{
    return m_aliases;
}

/**
 * @short Icon of chat type.
 * @return icon of chat type
 *
 * Chat type icon. Icon is used in history window and as icon of chat windows.
 *
 * Icon for ChatTypeBuddy is 'kadu_icons/conference'.
 */
KaduIcon ChatTypeBuddy::icon() const
{
    return KaduIcon(QStringLiteral("kadu_icons/conference"));
}

/**
 * @short Window role for this chat type.
 * @return window role for this chat type.
 *
 * Kadu window role for this chat type.
 * For aggregate the role is "kadu-chat-aggregate".
 */
QString ChatTypeBuddy::windowRole() const
{
    return QStringLiteral("kadu-chat-aggregate");
}

/**
 * @short Creates new ChatDetailsBuddy object for given chat type.
 * @return new ChatDetailsBuddy object for given chat type
 *
 * Creates new @link ChatDetailsBuddy @endlink object for
 * given @link Chat @endlink (@link ChatShared @endlink).
 */
ChatDetails *ChatTypeBuddy::createChatDetails(ChatShared *chatData) const
{
    return m_injectedFactory->makeInjected<ChatDetailsBuddy>(chatData);
}

ChatEditWidget *ChatTypeBuddy::createEditWidget(const Chat &chat, QWidget *parent) const
{
    Q_UNUSED(chat);
    Q_UNUSED(parent);

    return 0;
}
