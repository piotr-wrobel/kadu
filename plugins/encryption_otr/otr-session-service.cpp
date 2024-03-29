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

extern "C" {
#include <libotr/proto.h>
#include <libotr/message.h>
}

#include "accounts/account.h"
#include "chat/chat-manager.h"
#include "chat/chat-storage.h"
#include "chat/chat.h"
#include "chat/type/chat-type-contact.h"
#include "html/html-conversion.h"
#include "html/html-string.h"
#include "message/message-manager.h"

#include "otr-app-ops-service.h"
#include "otr-op-data-factory.h"
#include "otr-op-data.h"
#include "otr-policy-service.h"
#include "otr-policy.h"
#include "otr-trust-level-service.h"
#include "otr-user-state-service.h"

#include "otr-session-service.h"
#include "moc_otr-session-service.cpp"

void OtrSessionService::wrapperOtrGoneSecure(void *data, ConnContext *context)
{
    Q_UNUSED(context);

    OtrOpData *opData = static_cast<OtrOpData *>(data);
    if (opData->sessionService())
        emit opData->sessionService()->goneSecure(opData->contact());
}

void OtrSessionService::wrapperOtrGoneInsecure(void *data, ConnContext *context)
{
    Q_UNUSED(context);

    OtrOpData *opData = static_cast<OtrOpData *>(data);
    if (opData->sessionService())
        emit opData->sessionService()->goneInsecure(opData->contact());
}

void OtrSessionService::wrapperOtrStillSecure(void *data, ConnContext *context, int isReply)
{
    Q_UNUSED(context);
    Q_UNUSED(isReply);

    OtrOpData *opData = static_cast<OtrOpData *>(data);
    if (opData->sessionService())
        emit opData->sessionService()->stillSecure(opData->contact());
}

OtrSessionService::OtrSessionService()
{
}

OtrSessionService::~OtrSessionService()
{
}

void OtrSessionService::setAppOpsService(OtrAppOpsService *appOpsService)
{
    AppOpsService = appOpsService;
}

void OtrSessionService::setChatManager(ChatManager *chatManager)
{
    m_chatManager = chatManager;
}

void OtrSessionService::setChatStorage(ChatStorage *chatStorage)
{
    m_chatStorage = chatStorage;
}

void OtrSessionService::setMessageManager(MessageManager *messageManager)
{
    CurrentMessageManager = messageManager;
}

void OtrSessionService::setOpDataFactory(OtrOpDataFactory *opDataFactory)
{
    OpDataFactory = opDataFactory;
}

void OtrSessionService::setPolicyService(OtrPolicyService *policyService)
{
    PolicyService = policyService;
}

void OtrSessionService::setTrustLevelService(OtrTrustLevelService *trustLevelService)
{
    TrustLevelService = trustLevelService;
}

void OtrSessionService::setUserStateService(OtrUserStateService *userStateService)
{
    UserStateService = userStateService;
}

void OtrSessionService::startSession(const Contact &contact)
{
    if (!CurrentMessageManager || !PolicyService || !TrustLevelService)
        return;

    auto level = TrustLevelService->loadTrustLevelFromContact(contact);
    auto account = contact.contactAccount();
    auto otrPolicy = PolicyService->accountPolicy(account);
    auto message = normalizeHtml(
        HtmlString{QString::fromUtf8(otrl_proto_default_query_msg(qPrintable(account.id()), otrPolicy.toOtrPolicy()))});

    if (level == OtrTrustLevelService::TrustLevelNotPrivate)
        emit tryingToStartSession(contact);
    else
        emit tryingToRefreshSession(contact);

    CurrentMessageManager->sendMessage(
        ChatTypeContact::findChat(m_chatManager, m_chatStorage, contact, ActionCreateAndAdd), message, true);
}

void OtrSessionService::endSession(const Contact &contact)
{
    if (!AppOpsService || !OpDataFactory || !UserStateService)
        return;

    OtrOpData opData = OpDataFactory->opDataForContact(contact);
    otrl_message_disconnect_all_instances(
        UserStateService->userState(), AppOpsService->appOps(), &opData, qPrintable(contact.contactAccount().id()),
        qPrintable(contact.contactAccount().protocolName()), qPrintable(contact.id()));

    emit goneInsecure(contact);
}
