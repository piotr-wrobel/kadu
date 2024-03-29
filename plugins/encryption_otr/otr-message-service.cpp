/*
 * %kadu copyright begin%
 * Copyright 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "accounts/account.h"
#include "chat/chat-manager.h"
#include "chat/chat-service-repository.h"
#include "chat/chat-storage.h"
#include "chat/chat.h"
#include "chat/type/chat-type-contact.h"
#include "message/message-manager.h"
#include "protocols/services/chat-service.h"

#include "otr-op-data.h"

#include "otr-message-service.h"
#include "moc_otr-message-service.cpp"

void OtrMessageService::wrapperOtrInjectMessage(
    void *data, const char *accountName, const char *protocol, const char *recipient, const char *message)
{
    Q_UNUSED(accountName);
    Q_UNUSED(protocol);
    Q_UNUSED(recipient);

    OtrOpData *opData = static_cast<OtrOpData *>(data);
    if (opData->messageService())
        return opData->messageService()->injectMessage(opData->contact(), QByteArray(message));
}

int OtrMessageService::wrapperOtrMaxMessageSize(void *data, ConnContext *context)
{
    Q_UNUSED(context);

    OtrOpData *opData = static_cast<OtrOpData *>(data);
    if (opData->messageService())
        return opData->messageService()->maxMessageSize(opData->contact().contactAccount());
    else
        return 0;
}

const char *OtrMessageService::wrapperOtrResentMessagePrefix(void *data, ConnContext *context)
{
    Q_UNUSED(data);
    Q_UNUSED(context);

    OtrOpData *opData = static_cast<OtrOpData *>(data);
    if (opData->messageService())
        return strdup(qPrintable(opData->messageService()->resentMessagePrefix()));
    else
        return 0;
}

void OtrMessageService::wrapperOtrResentMessagePrefixFree(void *data, const char *prefix)
{
    Q_UNUSED(data);

    free(const_cast<char *>(prefix));
}

OtrMessageService::OtrMessageService()
{
}

OtrMessageService::~OtrMessageService()
{
}

void OtrMessageService::setChatManager(ChatManager *chatManager)
{
    m_chatManager = chatManager;
}

void OtrMessageService::setChatServiceRepository(ChatServiceRepository *chatServiceRepository)
{
    m_chatServiceRepository = chatServiceRepository;
}

void OtrMessageService::setChatStorage(ChatStorage *chatStorage)
{
    m_chatStorage = chatStorage;
}

void OtrMessageService::setMessageManager(MessageManager *messageManager)
{
    CurrentMessageManager = messageManager;
}

void OtrMessageService::injectMessage(const Contact &contact, const QByteArray &message) const
{
    if (!CurrentMessageManager)
        return;

    Chat chat = ChatTypeContact::findChat(m_chatManager, m_chatStorage, contact, ActionCreateAndAdd);
    CurrentMessageManager.data()->sendRawMessage(chat, message);
}

int OtrMessageService::maxMessageSize(const Account &account) const
{
    auto chatService = m_chatServiceRepository->chatService(account);
    if (!chatService)
        return 0;
    return chatService->maxMessageLength();
}

QString OtrMessageService::resentMessagePrefix() const
{
    return tr("[resent]");
}
