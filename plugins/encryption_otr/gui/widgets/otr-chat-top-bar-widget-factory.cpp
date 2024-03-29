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

#include "chat/chat.h"
#include "chat/type/chat-type-manager.h"
#include "contacts/contact-set.h"
#include "plugin/plugin-injected-factory.h"

#include "gui/widgets/otr-chat-top-bar-widget.h"
#include "gui/windows/otr-peer-identity-verification-window-repository.h"
#include "otr-session-service.h"
#include "otr-trust-level-service.h"

#include "otr-chat-top-bar-widget-factory.h"
#include "moc_otr-chat-top-bar-widget-factory.cpp"

OtrChatTopBarWidgetFactory::OtrChatTopBarWidgetFactory()
{
}

OtrChatTopBarWidgetFactory::~OtrChatTopBarWidgetFactory()
{
}

void OtrChatTopBarWidgetFactory::setChatTypeManager(ChatTypeManager *chatTypeManager)
{
    m_chatTypeManager = chatTypeManager;
}

void OtrChatTopBarWidgetFactory::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void OtrChatTopBarWidgetFactory::setPeerIdentityVerificationWindowRepository(
    OtrPeerIdentityVerificationWindowRepository *peerIdentityVerificationWindowRepository)
{
    PeerIdentityVerificationWindowRepository = peerIdentityVerificationWindowRepository;
}

void OtrChatTopBarWidgetFactory::setSessionService(OtrSessionService *sessionService)
{
    SessionService = sessionService;
}

void OtrChatTopBarWidgetFactory::setTrustLevelService(OtrTrustLevelService *trustLevelService)
{
    TrustLevelService = trustLevelService;
}

QWidget *OtrChatTopBarWidgetFactory::createWidget(const Chat &chat, QWidget *parent)
{
    auto chatType = m_chatTypeManager->chatType(chat.type());
    if (chatType->name() != "Contact")
        return 0;

    auto result = m_pluginInjectedFactory->makeInjected<OtrChatTopBarWidget>(chat.contacts().toContact(), parent);
    result->setTrustLevelService(TrustLevelService.data());

    if (SessionService)
    {
        connect(result, SIGNAL(startSession(Contact)), SessionService.data(), SLOT(startSession(Contact)));
        connect(result, SIGNAL(endSession(Contact)), SessionService.data(), SLOT(endSession(Contact)));
    }

    if (PeerIdentityVerificationWindowRepository)
        connect(
            result, SIGNAL(verifyPeerIdentity(Contact)), PeerIdentityVerificationWindowRepository.data(),
            SLOT(showVerificationWindow(Contact)));

    connect(result, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
    Widgets.append(result);

    return result;
}

void OtrChatTopBarWidgetFactory::widgetDestroyed(QObject *widget)
{
    OtrChatTopBarWidget *otrWidget = static_cast<OtrChatTopBarWidget *>(widget);
    Widgets.removeAll(otrWidget);
}
