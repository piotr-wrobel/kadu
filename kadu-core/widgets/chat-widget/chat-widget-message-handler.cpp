/*
 * %kadu copyright begin%
 * Copyright 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "chat-widget-message-handler.h"
#include "moc_chat-widget-message-handler.cpp"

#include "chat/buddy-chat-manager.h"
#include "configuration/configuration-api.h"
#include "configuration/deprecated-configuration-api.h"
#include "message/message-manager.h"
#include "message/sorted-messages.h"
#include "message/unread-message-repository.h"
#include "notification/silent-mode-service.h"
#include "protocols/protocol.h"
#include "status/status-type-data.h"
#include "status/status-type-manager.h"
#include "widgets/chat-widget/chat-widget-activation-service.h"
#include "widgets/chat-widget/chat-widget-manager.h"
#include "widgets/chat-widget/chat-widget-repository.h"
#include "widgets/chat-widget/chat-widget.h"
#include "windows/kadu-window-service.h"
#include "windows/kadu-window.h"

#include <QtWidgets/QApplication>

ChatWidgetMessageHandler::ChatWidgetMessageHandler(QObject *parent) : QObject{parent}
{
}

ChatWidgetMessageHandler::~ChatWidgetMessageHandler()
{
}

void ChatWidgetMessageHandler::setBuddyChatManager(BuddyChatManager *buddyChatManager)
{
    m_buddyChatManager = buddyChatManager;
}

void ChatWidgetMessageHandler::setChatWidgetActivationService(ChatWidgetActivationService *chatWidgetActivationService)
{
    m_chatWidgetActivationService = chatWidgetActivationService;
}

void ChatWidgetMessageHandler::setChatWidgetManager(ChatWidgetManager *chatWidgetManager)
{
    m_chatWidgetManager = chatWidgetManager;
}

void ChatWidgetMessageHandler::setChatWidgetRepository(ChatWidgetRepository *chatWidgetRepository)
{
    m_chatWidgetRepository = chatWidgetRepository;
}

void ChatWidgetMessageHandler::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void ChatWidgetMessageHandler::setKaduWindowService(KaduWindowService *kaduWindowService)
{
    m_kaduWindowService = kaduWindowService;
}

void ChatWidgetMessageHandler::setMessageManager(MessageManager *messageManager)
{
    m_messageManager = messageManager;
}

void ChatWidgetMessageHandler::setSilentModeService(SilentModeService *silentModeService)
{
    m_silentModeService = silentModeService;
}

void ChatWidgetMessageHandler::setStatusTypeManager(StatusTypeManager *statusTypeManager)
{
    m_statusTypeManager = statusTypeManager;
}

void ChatWidgetMessageHandler::setUnreadMessageRepository(UnreadMessageRepository *unreadMessageRepository)
{
    m_unreadMessageRepository = unreadMessageRepository;
}

void ChatWidgetMessageHandler::init()
{
    connect(
        m_chatWidgetActivationService, SIGNAL(chatWidgetActivated(ChatWidget *)), this,
        SLOT(chatWidgetActivated(ChatWidget *)));

    connect(
        m_chatWidgetRepository.data(), SIGNAL(chatWidgetAdded(ChatWidget *)), this,
        SLOT(chatWidgetAdded(ChatWidget *)));
    connect(
        m_chatWidgetRepository.data(), SIGNAL(chatWidgetRemoved(ChatWidget *)), this,
        SLOT(chatWidgetRemoved(ChatWidget *)));

    for (auto chatWidget : m_chatWidgetRepository.data())
        chatWidgetAdded(chatWidget);

    // some other messageReceived slot may check if message chat is open and this
    // slot can change this value, so let all other messageReceived be executed before this
    connect(
        m_messageManager.data(), SIGNAL(messageReceived(Message)), this, SLOT(messageReceived(Message)),
        Qt::QueuedConnection);
    connect(
        m_messageManager.data(), SIGNAL(messageSent(Message)), this, SLOT(messageSent(Message)), Qt::QueuedConnection);
}

void ChatWidgetMessageHandler::setChatWidgetMessageHandlerConfiguration(
    ChatWidgetMessageHandlerConfiguration chatWidgetMessageHandlerConfiguration)
{
    m_chatWidgetMessageHandlerConfiguration = chatWidgetMessageHandlerConfiguration;
}

void ChatWidgetMessageHandler::chatWidgetAdded(ChatWidget *chatWidget)
{
    appendAllUnreadMessages(chatWidget);
}

void ChatWidgetMessageHandler::chatWidgetRemoved(ChatWidget *chatWidget)
{
    auto chat = chatWidget->chat();
    chat.removeProperty("message:unreadMessagesAppended");
}

void ChatWidgetMessageHandler::chatWidgetActivated(ChatWidget *chatWidget)
{
    appendAllUnreadMessages(chatWidget);
}

void ChatWidgetMessageHandler::appendAllUnreadMessages(ChatWidget *chatWidget)
{
    if (!m_unreadMessageRepository)
        return;

    auto chat = chatWidget->chat();
    auto unreadMessagesAppended = chat.property("message:unreadMessagesAppended", false).toBool();

    auto messages = unreadMessagesAppended ? m_unreadMessageRepository.data()->unreadMessagesForChat(chat)
                                           : loadAllUnreadMessages(chat);
    auto chatIsActive =
        m_chatWidgetActivationService ? m_chatWidgetActivationService.data()->isChatWidgetActive(chatWidget) : false;
    if (chatIsActive)
        m_unreadMessageRepository.data()->markMessagesAsRead(messages);

    if (!unreadMessagesAppended)
    {
        chatWidget->addMessages(messages);
        chat.addProperty("message:unreadMessagesAppended", true, CustomProperties::NonStorable);
    }
}

SortedMessages ChatWidgetMessageHandler::loadAllUnreadMessages(const Chat &chat) const
{
    // TODO: BuddyChatManager cannot be injected here, because it crashes, find out why
    auto buddyChat = m_buddyChatManager->buddyChat(chat);
    auto unreadChat = buddyChat ? buddyChat : chat;
    return m_unreadMessageRepository.data()->unreadMessagesForChat(unreadChat);
}

void ChatWidgetMessageHandler::messageReceived(const Message &message)
{
    if (!m_chatWidgetRepository)
        return;

    auto chat = message.messageChat();
    auto chatWidget = m_chatWidgetRepository.data()->widgetForChat(chat);
    auto chatIsActive =
        m_chatWidgetActivationService ? m_chatWidgetActivationService.data()->isChatWidgetActive(chatWidget) : false;

    if (m_unreadMessageRepository && !chatIsActive)
        m_unreadMessageRepository.data()->addUnreadMessage(message);

    if (chatWidget)
    {
        chatWidget->addMessage(message);
        return;
    }

    if (shouldOpenChatWidget(chat))
    {
        auto activation = m_chatWidgetMessageHandlerConfiguration.openChatOnMessageMinimized()
                              ? OpenChatActivation::Minimize
                              : OpenChatActivation::Activate;
        m_chatWidgetManager.data()->openChat(chat, activation);
    }
    else
    {
#ifdef Q_OS_WIN
        if (!m_configuration->deprecatedApi()->readBoolEntry("General", "HideMainWindowFromTaskbar"))
            qApp->alert(m_kaduWindowService->kaduWindow());
#else
        qApp->alert(m_kaduWindowService->kaduWindow());
#endif
    }
}

bool ChatWidgetMessageHandler::shouldOpenChatWidget(const Chat &chat) const
{
    if (!m_chatWidgetMessageHandlerConfiguration.openChatOnMessage())
        return false;

    auto silentMode = m_silentModeService->isSilentOrAutoSilent();
    if (silentMode)
        return false;

    auto handler = chat.chatAccount().protocolHandler();
    if (!handler)
        return false;

    if (m_chatWidgetMessageHandlerConfiguration.openChatOnMessageOnlyWhenOnline())
        return StatusTypeGroup::Online == m_statusTypeManager->statusTypeData(handler->status().type()).typeGroup();
    else
        return true;
}

void ChatWidgetMessageHandler::messageSent(const Message &message)
{
    if (!m_chatWidgetRepository)
        return;

    auto chat = message.messageChat();
    auto chatWidget = m_chatWidgetRepository.data()->widgetForChat(chat);
    if (chatWidget)
        chatWidget->addMessage(message);
}
