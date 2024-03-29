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
#include "contacts/contact.h"

#include "otr-op-data.h"
#include "otr-policy.h"

#include "otr-policy-service.h"
#include "moc_otr-policy-service.cpp"

OtrlPolicy OtrPolicyService::wrapperOtrPolicy(void *data, ConnContext *context)
{
    Q_UNUSED(context);

    OtrOpData *opData = static_cast<OtrOpData *>(data);
    if (opData->policyService())
        return opData->policyService()->contactPolicy(opData->contact()).toOtrPolicy();
    else
        return OtrPolicy::PolicyManual.toOtrPolicy();
}

OtrPolicyService::OtrPolicyService()
{
}

OtrPolicyService::~OtrPolicyService()
{
}

void OtrPolicyService::setAccountPolicy(const Account &account, const OtrPolicy &policy)
{
    account.addProperty("otr:policy", policy.toString(), CustomProperties::Storable);
}

OtrPolicy OtrPolicyService::accountPolicy(const Account &account) const
{
    OtrPolicy fromAccount = OtrPolicy::fromString(account.property("otr:policy", QVariant()).toString());
    if (OtrPolicy::PolicyUndefined == fromAccount)
        return OtrPolicy::PolicyManual;
    else
        return fromAccount;
}

void OtrPolicyService::setBuddyPolicy(const Buddy &buddy, const OtrPolicy &policy)
{
    buddy.addProperty("otr:policy", policy.toString(), CustomProperties::Storable);
}

OtrPolicy OtrPolicyService::buddyPolicy(const Buddy &buddy) const
{
    return OtrPolicy::fromString(buddy.property("otr:policy", QVariant()).toString());
}

OtrPolicy OtrPolicyService::contactPolicy(const Contact &contact) const
{
    OtrPolicy fromBuddy = buddyPolicy(contact.ownerBuddy());
    if (OtrPolicy::PolicyUndefined == fromBuddy)
        return accountPolicy(contact.contactAccount());
    else
        return fromBuddy;
}
