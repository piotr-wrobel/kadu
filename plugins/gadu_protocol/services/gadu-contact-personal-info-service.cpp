/*
 * %kadu copyright begin%
 * Copyright 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "buddies/buddy-storage.h"
#include "contacts/contact-storage.h"
#include "misc/misc.h"

#include "helpers/gadu-protocol-helper.h"
#include "server/gadu-connection.h"
#include "server/gadu-writable-session-token.h"

#include "gadu-contact-personal-info-service.h"
#include "moc_gadu-contact-personal-info-service.cpp"

GaduContactPersonalInfoService::GaduContactPersonalInfoService(Account account, QObject *parent)
        : ContactPersonalInfoService(account, parent), FetchSeq(0)
{
}

void GaduContactPersonalInfoService::setBuddyStorage(BuddyStorage *buddyStorage)
{
    m_buddyStorage = buddyStorage;
}

void GaduContactPersonalInfoService::setContactStorage(ContactStorage *contactStorage)
{
    m_contactStorage = contactStorage;
}

void GaduContactPersonalInfoService::setConnection(GaduConnection *connection)
{
    Connection = connection;
}

void GaduContactPersonalInfoService::handleEventPubdir50Read(struct gg_event *e)
{
    gg_pubdir50_t res = e->event.pubdir50;

    if (FetchSeq != res->seq)
        return;

    int count = gg_pubdir50_count(res);
    if (1 != count)
    {
        emit personalInfoAvailable(Buddy::null);
        return;
    }

    Buddy result = GaduProtocolHelper::searchResultToBuddy(m_buddyStorage, m_contactStorage, account(), res, 0);
    emit personalInfoAvailable(result);
}

void GaduContactPersonalInfoService::fetchPersonalInfo(Contact contact)
{
    if (!Connection || !Connection.data()->hasSession())
        return;

    Id = contact.id();
    gg_pubdir50_t req = gg_pubdir50_new(GG_PUBDIR50_SEARCH);
    gg_pubdir50_add(req, GG_PUBDIR50_UIN, Id.toUtf8().constData());

    auto writableSessionToken = Connection.data()->writableSessionToken();
    FetchSeq = gg_pubdir50(writableSessionToken.rawSession(), req);
    // gg_pubdir50_free(req);
}
