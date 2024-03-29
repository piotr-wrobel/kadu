/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "buddy-contacts-table-item.h"
#include "moc_buddy-contacts-table-item.cpp"

#include "contacts/contact-manager.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol.h"
#include "roster/roster-entry-state.h"
#include "roster/roster-entry.h"

BuddyContactsTableItem::BuddyContactsTableItem(ContactManager *contactManager, Contact contact, QObject *parent)
        : QObject{parent}, m_contactManager{contactManager}
{
    ItemContact = contact;
    ItemContactPriority = contact.priority();
    ItemAccount = contact.contactAccount();
    Id = contact.id();
    RosterDetached = !contact.isNull() ? (contact.rosterEntry()->state() == RosterEntryState::Detached) : false;
    Action = ItemEdit;
}

void BuddyContactsTableItem::setItemContactPriority(int itemContactPriority)
{
    if (ItemContactPriority != itemContactPriority)
    {
        ItemContactPriority = itemContactPriority;
        emit updated(this);
    }
}

void BuddyContactsTableItem::setItemAccount(Account account)
{
    if (ItemAccount != account)
    {
        ItemAccount = account;
        emit updated(this);
    }
}

void BuddyContactsTableItem::setId(const QString &id)
{
    if (Id != id)
    {
        Id = id;
        emit updated(this);
    }
}

void BuddyContactsTableItem::setRosterDetached(bool rosterDetached)
{
    if (RosterDetached != rosterDetached)
    {
        RosterDetached = rosterDetached;
        emit updated(this);
    }
}

void BuddyContactsTableItem::setAction(BuddyContactsTableItem::ItemAction action)
{
    if (Action != action)
    {
        Action = action;
        emit updated(this);
    }
}

void BuddyContactsTableItem::setDetachedBuddyName(const QString &detachedBuddyName)
{
    if (DetachedBuddyName != detachedBuddyName)
    {
        DetachedBuddyName = detachedBuddyName;
        emit updated(this);
    }
}

bool BuddyContactsTableItem::isValid() const
{
    if (ItemRemove == Action)
        return true;

    if (ItemDetach == Action)
        return !DetachedBuddyName.isEmpty();

    if (ItemAdd == Action)
        return isAddValid();

    return isEditValid();
}

bool BuddyContactsTableItem::isAddValid() const
{
    if (ItemContact)
        return false;

    if (!ItemAccount)
        return false;

    if (Id.isEmpty())
        return false;

    Protocol *handler = ItemAccount.protocolHandler();
    if (!handler)
        return false;

    if (handler->protocolFactory()->validateId(Id) != QValidator::Acceptable)
        return false;

    // allow contacts without buddy or new ones
    Contact contact = m_contactManager->byId(ItemAccount, Id, ActionReturnNull);
    return contact.ownerBuddy().isAnonymous();
}

bool BuddyContactsTableItem::isEditValid() const
{
    if (!ItemContact)
        return false;

    if (!ItemAccount)
        return false;

    if (Id.isEmpty())
        return false;

    Protocol *handler = ItemAccount.protocolHandler();
    if (!handler)
        return true;   // make it user responsibility

    if (handler->protocolFactory()->validateId(Id) != QValidator::Acceptable)
        return false;

    if (ItemAccount != ItemContact.contactAccount() || Id != ItemContact.id())
    {
        // allow contacts without buddy or new ones
        Contact contact = m_contactManager->byId(ItemAccount, Id, ActionReturnNull);
        return contact.ownerBuddy().isAnonymous();
    }

    return true;
}
