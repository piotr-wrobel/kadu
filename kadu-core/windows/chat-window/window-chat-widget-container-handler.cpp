/*
 * %kadu copyright begin%
 * Copyright 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "window-chat-widget-container-handler.h"
#include "moc_window-chat-widget-container-handler.cpp"

#include "activate.h"
#include "core/injected-factory.h"
#include "widgets/chat-widget/chat-widget-manager.h"
#include "widgets/chat-widget/chat-widget.h"
#include "windows/chat-window/chat-window-repository.h"
#include "windows/chat-window/chat-window.h"

#include <QtWidgets/QApplication>

WindowChatWidgetContainerHandler::WindowChatWidgetContainerHandler(QObject *parent) : ChatWidgetContainerHandler{parent}
{
}

WindowChatWidgetContainerHandler::~WindowChatWidgetContainerHandler()
{
}

void WindowChatWidgetContainerHandler::setChatWindowRepository(ChatWindowRepository *chatWindowRepository)
{
    m_chatWindowRepository = chatWindowRepository;
}

void WindowChatWidgetContainerHandler::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void WindowChatWidgetContainerHandler::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

bool WindowChatWidgetContainerHandler::acceptChat(Chat chat) const
{
    return chat;
}

ChatWidget *WindowChatWidgetContainerHandler::addChat(Chat chat, OpenChatActivation activation)
{
    if (!acceptChat(chat))
        return nullptr;

    auto chatWindow = m_chatWindowRepository.data()->windowForChat(chat);
    if (!chatWindow)
    {
        chatWindow = m_injectedFactory->makeInjected<ChatWindow>(chat);
        if (!chatWindow)
            return nullptr;

        m_chatWindowRepository.data()->addChatWindow(chatWindow);
        connect(chatWindow, SIGNAL(activated(ChatWindow *)), this, SLOT(chatWindowActivated(ChatWindow *)));
    }

    switch (activation)
    {
    case OpenChatActivation::Minimize:
        chatWindow->showMinimized();
        break;
    default:
        chatWindow->show();
        break;
    }

    if (chat.unreadMessagesCount() != 0)
        qApp->alert(chatWindow);

    return chatWindow->chatWidget();
}

void WindowChatWidgetContainerHandler::removeChat(Chat chat)
{
    if (!chat || !m_chatWindowRepository)
        return;

    auto chatWindow = m_chatWindowRepository.data()->windowForChat(chat);
    chatWindow->deleteLater();
}

bool WindowChatWidgetContainerHandler::isChatWidgetActive(ChatWidget *chatWidget)
{
    if (!chatWidget || !m_chatWindowRepository)
        return false;

    auto chatWindow = m_chatWindowRepository.data()->windowForChat(chatWidget->chat());
    return chatWindow ? chatWindow->isChatWidgetActive(chatWidget) : false;
}

void WindowChatWidgetContainerHandler::tryActivateChatWidget(ChatWidget *chatWidget)
{
    if (!chatWidget || !m_chatWindowRepository)
        return;

    auto chatWindow = m_chatWindowRepository.data()->windowForChat(chatWidget->chat());
    if (chatWindow)
        _activateWindow(m_configuration, chatWindow);
}

void WindowChatWidgetContainerHandler::tryMinimizeChatWidget(ChatWidget *chatWidget)
{
    if (!chatWidget || !m_chatWindowRepository)
        return;

    auto chatWindow = m_chatWindowRepository.data()->windowForChat(chatWidget->chat());
    if (chatWindow)
        chatWindow->showMinimized();
}

void WindowChatWidgetContainerHandler::chatWindowActivated(ChatWindow *chatWindow)
{
    if (chatWindow && chatWindow->chatWidget())
        emit chatWidgetActivated(chatWindow->chatWidget());
}
