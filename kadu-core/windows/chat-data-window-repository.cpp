/*
 * %kadu copyright begin%
 * Copyright 2013 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "core/injected-factory.h"
#include "windows/chat-data-window.h"

#include "chat-data-window-repository.h"
#include "moc_chat-data-window-repository.cpp"

ChatDataWindowRepository::ChatDataWindowRepository(QObject *parent) : QObject(parent)
{
}

ChatDataWindowRepository::~ChatDataWindowRepository()
{
}

void ChatDataWindowRepository::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

ChatDataWindow *ChatDataWindowRepository::windowForChat(const Chat &chat)
{
    if (Windows.contains(chat))
        return Windows.value(chat);

    auto result = m_injectedFactory->makeInjected<ChatDataWindow>(chat);
    connect(result, SIGNAL(destroyed(Chat)), this, SLOT(windowDestroyed(Chat)));
    Windows.insert(chat, result);

    return result;
}

const QMap<Chat, ChatDataWindow *> &ChatDataWindowRepository::windows() const
{
    return Windows;
}

void ChatDataWindowRepository::windowDestroyed(const Chat &chat)
{
    Windows.remove(chat);
}

void ChatDataWindowRepository::showChatWindow(const Chat &chat)
{
    ChatDataWindow *window = windowForChat(chat);
    if (window)
    {
        window->show();
        window->raise();
    }
}
