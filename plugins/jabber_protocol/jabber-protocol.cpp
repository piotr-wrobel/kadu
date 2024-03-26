/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "jabber-protocol.h"
#include "jabber-protocol.moc"

#include "actions/jabber-actions.h"
#include "actions/jabber-protocol-menu-manager.h"
#include "gtalk-protocol-factory.h"
#include "jabber-id-validator.h"
#include "jabber-protocol-factory.h"
#include "jabber-url-handler.h"
#include "jid.h"
#include "open-chat-with/jabber-open-chat-with-runner.h"
#include "qxmpp/jabber-register-extension.h"
#include "qxmpp/jabber-roster-extension.h"
#include "qxmpp/jabber-ssl-handler.h"
#include "services/jabber-change-password-service.h"
#include "services/jabber-chat-service.h"
#include "services/jabber-chat-state-service.h"
#include "services/jabber-contact-avatar-service.h"
#include "services/jabber-error-service.h"
#include "services/jabber-file-transfer-service.h"
#include "services/jabber-presence-service.h"
#include "services/jabber-register-account-service.h"
#include "services/jabber-resource-service.h"
#include "services/jabber-room-chat-service.h"
#include "services/jabber-roster-service.h"
#include "services/jabber-stream-debug-service.h"
#include "services/jabber-subscription-service.h"
#include "services/jabber-vcard-service.h"

#include "avatars/aggregated-account-avatar-service.h"
#include "avatars/aggregated-contact-avatar-service.h"
#include "buddies/buddy-manager.h"
#include "buddies/group-manager.h"
#include "chat/chat-manager.h"
#include "chat/chat-service-repository.h"
#include "chat/chat-state-service-repository.h"
#include "contacts/contact-manager.h"
#include "core/version-service.h"
#include "misc/memory.h"
#include "os/generic/system-info.h"
#include "plugin/plugin-injected-factory.h"
#include "protocols/protocols-manager.h"
#include "status/status-type-manager.h"
#include "url-handlers/url-handler-manager.h"
#include "windows/message-dialog.h"
#include "windows/open-chat-with/open-chat-with-runner-manager.h"

#include <QtCore/QCoreApplication>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QSslSocket>
#include <QXmppQt5/QXmppClient.h>
#include <QXmppQt5/QXmppMucManager.h>
#include <QXmppQt5/QXmppRosterManager.h>
#include <QXmppQt5/QXmppTransferManager.h>
#include <QXmppQt5/QXmppVCardManager.h>
#include <QXmppQt5/QXmppVersionManager.h>

JabberProtocol::JabberProtocol(Account account, ProtocolFactory *factory) : Protocol{account, factory}
{
}

JabberProtocol::~JabberProtocol()
{
    logout();

    OpenChatWithRunnerManager::instance()->unregisterRunner(m_jabberOpenChatWithRunner);
    delete m_jabberOpenChatWithRunner;
    m_jabberOpenChatWithRunner = 0;

    m_chatStateServiceRepository->removeChatStateService(m_chatStateService);
    m_chatServiceRepository->removeChatService(m_chatService);
    m_aggregatedContactAvatarService->remove(m_contactAvatarService);
    m_aggregatedAccountAvatarService->remove(m_accountAvatarService);
}

void JabberProtocol::setChatServiceRepository(ChatServiceRepository *chatServiceRepository)
{
    m_chatServiceRepository = chatServiceRepository;
}

void JabberProtocol::setAggregatedAccountAvatarService(AggregatedAccountAvatarService *aggregatedAccountAvatarService)
{
    m_aggregatedAccountAvatarService = aggregatedAccountAvatarService;
}

void JabberProtocol::setAggregatedContactAvatarService(AggregatedContactAvatarService *aggregatedContactAvatarService)
{
    m_aggregatedContactAvatarService = aggregatedContactAvatarService;
}

void JabberProtocol::setChatStateServiceRepository(ChatStateServiceRepository *chatStateServiceRepository)
{
    m_chatStateServiceRepository = chatStateServiceRepository;
}

void JabberProtocol::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void JabberProtocol::setSystemInfo(SystemInfo *systemInfo)
{
    m_systemInfo = systemInfo;
}

void JabberProtocol::setVersionService(VersionService *versionService)
{
    m_versionService = versionService;
}

void JabberProtocol::init()
{
    connect(account(), SIGNAL(updated()), this, SLOT(updatePresence()), Qt::UniqueConnection);

    m_presenceService = pluginInjectedFactory()->makeInjected<JabberPresenceService>(this);
    m_errorService = new JabberErrorService{this};

    m_client = new QXmppClient{this};
    connect(m_client, SIGNAL(connected()), this, SLOT(connectedToServer()));
    connect(m_client, SIGNAL(disconnected()), this, SLOT(disconenctedFromServer()));
    connect(m_client, SIGNAL(error(QXmppClient::Error)), this, SLOT(error(QXmppClient::Error)));
    connect(m_client, SIGNAL(presenceReceived(QXmppPresence)), this, SLOT(presenceReceived(QXmppPresence)));

    pluginInjectedFactory()->makeInjected<JabberSslHandler>(
        m_client, [&]() { emit stateMachineSslErrorResolved(); }, [&]() { emit stateMachineSslErrorNotResolved(); });

    m_registerExtension = std::make_unique<JabberRegisterExtension>();
    m_rosterExtension = std::make_unique<JabberRosterExtension>();
    m_mucManager = std::make_unique<QXmppMucManager>();
    m_transferManager = std::make_unique<QXmppTransferManager>();

    m_rosterExtension->setJabberErrorService(m_errorService);

    m_client->addExtension(m_registerExtension.get());
    m_client->insertExtension(0, m_rosterExtension.get());
    m_client->addExtension(m_mucManager.get());
    m_client->addExtension(m_transferManager.get());

    m_changePasswordService = new JabberChangePasswordService{m_registerExtension.get(), this};
    m_changePasswordService->setErrorService(m_errorService);

    m_resourceService = pluginInjectedFactory()->makeInjected<JabberResourceService>(this);

    m_roomChatService =
        pluginInjectedFactory()->makeInjected<JabberRoomChatService>(m_client, m_mucManager.get(), account(), this);

    m_chatStateService = pluginInjectedFactory()->makeInjected<JabberChatStateService>(m_client, account(), this);
    m_chatStateService->setResourceService(m_resourceService);

    m_accountAvatarService = pluginInjectedFactory()->makeInjected<JabberAccountAvatarService>(account(), this);

    m_chatService = pluginInjectedFactory()->makeInjected<JabberChatService>(m_client, account(), this);
    m_chatService->setChatStateService(m_chatStateService);
    m_chatService->setResourceService(m_resourceService);
    m_chatService->setRoomChatService(m_roomChatService);

    m_contactPersonalInfoService =
        pluginInjectedFactory()->makeInjected<JabberContactPersonalInfoService>(account(), this);
    m_personalInfoService = pluginInjectedFactory()->makeInjected<JabberPersonalInfoService>(account(), this);
    m_streamDebugService = new JabberStreamDebugService{m_client, this};

    m_fileTransferService =
        pluginInjectedFactory()->makeInjected<JabberFileTransferService>(m_transferManager.get(), account(), this);
    m_fileTransferService->setResourceService(m_resourceService);

    m_vcardService = new JabberVCardService{&m_client->vCardManager(), this};
    m_contactAvatarService =
        pluginInjectedFactory()->makeInjected<JabberContactAvatarService>(m_client, m_vcardService, account(), this);

    m_accountAvatarService->setVCardService(m_vcardService);
    m_contactPersonalInfoService->setVCardService(m_vcardService);
    m_personalInfoService->setVCardService(m_vcardService);

    auto contacts = contactManager()->contacts(account(), ContactManager::ExcludeAnonymous);
    auto rosterService = pluginInjectedFactory()->makeInjected<JabberRosterService>(
        &m_client->rosterManager(), m_rosterExtension.get(), contacts, this);

    connect(rosterService, SIGNAL(rosterReady()), this, SLOT(rosterReady()));

    setRosterService(rosterService);

    m_subscriptionService =
        pluginInjectedFactory()->makeInjected<JabberSubscriptionService>(&m_client->rosterManager(), this);

    m_jabberOpenChatWithRunner = m_pluginInjectedFactory->makeInjected<JabberOpenChatWithRunner>(account());
    OpenChatWithRunnerManager::instance()->registerRunner(m_jabberOpenChatWithRunner);

    m_aggregatedAccountAvatarService->add(m_accountAvatarService);
    m_aggregatedContactAvatarService->add(m_contactAvatarService);
    m_chatServiceRepository->addChatService(m_chatService);
    m_chatStateServiceRepository->addChatStateService(m_chatStateService);
}

void JabberProtocol::rosterReady()
{
    /* Since we are online now, set initial presence. Don't do this
    * before the roster request or we will receive presence
    * information before we have updated our roster with actual
    * contacts from the server! (Iris won't forward presence
    * information in that case either). */
    sendStatusToServer();
}

/*
 * login procedure
 *
 * After calling login method we set up JabberClient that must call connectedToServer in order to inform
 * us that connection was established. Then we can tell this to state machine in Protocol class
 */
void JabberProtocol::login()
{
    auto accountData = JabberAccountData{account()};
    if (accountData.publishSystemInfo())
    {
        m_client->versionManager().setClientName("Kadu");
        m_client->versionManager().setClientVersion(m_versionService->version());
        m_client->versionManager().setClientOs(m_systemInfo->osFullName());
    }
    else
    {
        m_client->versionManager().setClientName(QString{});
        m_client->versionManager().setClientVersion(QString{});
        m_client->versionManager().setClientOs(QString{});
    }

    auto streamSecurityMode = QXmppConfiguration::StreamSecurityMode{};
    switch (accountData.encryptionMode())
    {
    case JabberAccountData::Encryption_Auto:
        streamSecurityMode = QXmppConfiguration::StreamSecurityMode::TLSEnabled;
        break;
    case JabberAccountData::Encryption_Yes:
        streamSecurityMode = QXmppConfiguration::StreamSecurityMode::TLSRequired;
        break;
    case JabberAccountData::Encryption_No:
        streamSecurityMode = QXmppConfiguration::StreamSecurityMode::TLSDisabled;
        break;
    case JabberAccountData::Encryption_Legacy:
        streamSecurityMode = QXmppConfiguration::StreamSecurityMode::LegacySSL;
        break;
    }

    auto useNonSASLAuthentication =
        accountData.plainAuthMode() == JabberAccountData::AllowPlain
            ? true
            : accountData.plainAuthMode() == JabberAccountData::JabberAccountData::AllowPlainOverTLS
                  ? QXmppConfiguration::StreamSecurityMode::TLSDisabled != streamSecurityMode
                  : false;

    auto jid = Jid::parse(account().id()).withResource(accountData.resource(*m_systemInfo));

    auto configuration = QXmppConfiguration{};
    configuration.setAutoAcceptSubscriptions(false);
    configuration.setAutoReconnectionEnabled(false);
    configuration.setIgnoreSslErrors(false);
    configuration.setJid(jid.full());
    configuration.setPassword(account().password());
    configuration.setStreamSecurityMode(streamSecurityMode);
    configuration.setUseNonSASLAuthentication(useNonSASLAuthentication);

    auto proxy = toQNetworkProxy(account().proxy());
    if (proxy.type() != QNetworkProxy::NoProxy)
        configuration.setNetworkProxy(proxy);

    if (accountData.useCustomHostPort())
    {
        configuration.setHost(accountData.customHost());
        configuration.setPort(accountData.customPort());
    }

    auto presence = m_presenceService->statusToPresence(status());
    presence.setPriority(accountData.priority());

    static_cast<JabberRosterService *>(rosterService())->prepareRoster();
    m_client->connectToServer(configuration, presence);
}

void JabberProtocol::connectedToServer()
{
    loggedIn();
}

void JabberProtocol::logout()
{
    auto logoutStatus = status();
    logoutStatus.setType(StatusType::Offline);
    m_client->setClientPresence(m_presenceService->statusToPresence(logoutStatus));
    m_client->disconnectFromServer();

    loggedOut();
}

void JabberProtocol::disconenctedFromServer()
{
    m_resourceService->clear();
}

void JabberProtocol::error(QXmppClient::Error error)
{
    auto errorMessage = QString{};
    switch (error)
    {
    case QXmppClient::Error::SocketError:
    {
        switch (m_client->socketError())
        {
        case QAbstractSocket::SslHandshakeFailedError:
            sslError();
            return;
        default:
            break;
        }
        break;
    }

    case QXmppClient::Error::XmppStreamError:
    {
        switch (m_client->xmppStreamError())
        {
        case QXmppStanza::Error::NotAuthorized:
            passwordRequired();
            return;
        case QXmppStanza::Error::Conflict:
            errorMessage = tr("Another client connected on the same resource.");
            setStatus(Status{}, SourceUser);
            break;
        default:
            break;
        }
        break;
    }

    default:
        break;
    }

    if (errorMessage.isEmpty())
        errorMessage = m_errorService->errorMessage(m_client, error);
    emit connectionError(account(), m_client->configuration().host(), errorMessage);
    connectionError();

    m_client->disconnectFromServer();
}

void JabberProtocol::updatePresence()
{
    sendStatusToServer();
}

void JabberProtocol::sendStatusToServer()
{
    if (!isConnected() && !isDisconnecting())
        return;

    auto presence = m_presenceService->statusToPresence(status());
    auto accountData = JabberAccountData{account()};
    presence.setPriority(accountData.priority());

    m_client->setClientPresence(presence);
    account().accountContact().setCurrentStatus(status());
}

void JabberProtocol::changePrivateMode()
{
    sendStatusToServer();
}

void JabberProtocol::presenceReceived(const QXmppPresence &presence)
{
    if (presence.isMucSupported())
        return;

    auto jid = Jid::parse(presence.from());
    auto id = jid.bare();
    auto contact = contactManager()->byId(account(), id, ActionReturnNull);
    if (!contact)
        return;

    auto status = m_presenceService->presenceToStatus(presence);
    if (status.type() != StatusType::Offline)
    {
        auto jabberResource = JabberResource{jid, presence.priority(), status};
        m_resourceService->updateResource(jabberResource);
    }
    else
    {
        if (contact.property("jabber:chat-resource", QString{}).toString() == jid.resource())
            contact.removeProperty("jabber:chat-resource");
        m_resourceService->removeResource(jid);
    }

    auto bestResource = m_resourceService->bestResource(id);
    auto statusToSet = bestResource.isEmpty() ? status : bestResource.status();

    auto oldStatus = contact.currentStatus();
    contact.setCurrentStatus(statusToSet);

    // see issue #2159 - we need a way to ignore first status of given contact
    if (contact.ignoreNextStatusChange())
        contact.setIgnoreNextStatusChange(false);
    else
        emit contactStatusChanged(contact, oldStatus);
}

QString JabberProtocol::statusPixmapPath()
{
    return QStringLiteral("xmpp");
}

JabberChangePasswordService *JabberProtocol::changePasswordService() const
{
    return m_changePasswordService;
}
