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
#include "contacts/contact-manager.h"

#include "otr-op-data.h"

#include "otr-is-logged-in-service.h"
#include "moc_otr-is-logged-in-service.cpp"

int OtrIsLoggedInService::wrapperOtrIsLoggedIn(
    void *data, const char *accountName, const char *protocol, const char *recipient)
{
    Q_UNUSED(accountName);
    Q_UNUSED(protocol);

    OtrOpData *opData = static_cast<OtrOpData *>(data);
    if (opData->isLoggedInService())
        return static_cast<int>(
            opData->isLoggedInService()->isLoggedIn(opData->contact().contactAccount(), QString::fromUtf8(recipient)));
    else
        return static_cast<int>(OtrIsLoggedInService::NotSure);
}

OtrIsLoggedInService::OtrIsLoggedInService(QObject *parent) : QObject{parent}
{
}

OtrIsLoggedInService::~OtrIsLoggedInService()
{
}

void OtrIsLoggedInService::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

OtrIsLoggedInService::IsLoggedInStatus
OtrIsLoggedInService::isLoggedIn(const Account &account, const QString &contactId)
{
    auto contact = m_contactManager->byId(account, contactId, ActionReturnNull);

    if (!contact)
        return NotSure;

    if (contact.currentStatus().isDisconnected())
        return NotLoggedIn;
    else
        return LoggedIn;
}
