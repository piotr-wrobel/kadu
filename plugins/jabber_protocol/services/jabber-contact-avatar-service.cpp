/*
 * %kadu copyright begin%
 * Copyright 2017 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "jabber-contact-avatar-service.h"
#include "moc_jabber-contact-avatar-service.cpp"

#include "jid.h"
#include "services/jabber-avatar-downloader.h"

#include "avatars/contact-avatar-id.h"
#include "contacts/contact-id.h"
#include "misc/memory.h"

#include <QXmppQt5/QXmppClient.h>
#include <QXmppQt5/QXmppRosterManager.h>

JabberContactAvatarService::JabberContactAvatarService(
    QXmppClient *client, JabberVCardService *vCardService, Account account, QObject *parent)
        : ContactAvatarService{account, parent}, m_client{client}, m_vCardService{vCardService}
{
    connect(
        &m_client->rosterManager(), &QXmppRosterManager::rosterReceived, this,
        &JabberContactAvatarService::rosterReceived);
    connect(m_client, &QXmppClient::presenceReceived, this, &JabberContactAvatarService::presenceReceived);
}

JabberContactAvatarService::~JabberContactAvatarService() = default;

void JabberContactAvatarService::download(const ContactAvatarId &id)
{
    auto avatarDownloader = make_owned<JabberAvatarDownloader>(id, m_vCardService, this);
    connect(avatarDownloader, &JabberAvatarDownloader::downloaded, this, &JabberContactAvatarService::downloaded);
}

void JabberContactAvatarService::rosterReceived()
{
    for (auto &&bareId : m_client->rosterManager().getRosterBareJids())
        for (auto &&presence : m_client->rosterManager().getAllPresencesForBareJid(bareId))
            presenceReceived(presence);
}

void JabberContactAvatarService::presenceReceived(const QXmppPresence &presence)
{
    auto jid = Jid::parse(presence.from());
    auto contactId = ContactId{jid.bare().toUtf8()};

    switch (presence.vCardUpdateType())
    {
    case QXmppPresence::VCardUpdateNoPhoto:
        emit removed(contactId);
        break;
    case QXmppPresence::VCardUpdateValidPhoto:
        emit available({contactId, presence.photoHash()});
        break;
    default:
        break;
    }
}
