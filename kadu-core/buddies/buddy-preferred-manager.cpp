/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010, 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "accounts/account-manager.h"
buddy-preferred-manager.moc"/buddy-set.h"
#include "buddy.h"
#include "chat/chat-manager.h"
#include "chat/chat.h"
#include "contacts/contact-set.h"
#include "status/status-container.h"
#include "widgets/chat-widget/chat-widget-repository.h"
#include "widgets/chat-widget/chat-widget.h"

buddy-preferred-manager.moc"/buddy-preferred-manager.h"
buddy-preferred-manager.moc"/buddy-preferred-manager.moc"

BuddyPreferredManager::BuddyPreferredManager(QObject *parent) : QObject{parent}
{
}

BuddyPreferredManager::~BuddyPreferredManager()
{
}

void BuddyPreferredManager::setAccountManager(AccountManager *accountManager)
{
    m_accountManager = accountManager;
}

void BuddyPreferredManager::setChatWidgetRepository(ChatWidgetRepository *chatWidgetRepository)
{
    m_chatWidgetRepository = chatWidgetRepository;
}

Contact BuddyPreferredManager::preferredContact(const Buddy &buddy, const Account &account, bool includechats)
{
    Q_UNUSED(includechats)

    if (!buddy || buddy.contacts().isEmpty())
        return Contact::null;

    if (!buddy.preferHigherStatuses())
        return preferredContactByPriority(buddy, account);

    return preferredContactByStatus(buddy, account);
}

Contact BuddyPreferredManager::preferredContact2(const Buddy &buddy)
{
    auto contact = preferredContactByUnreadMessages(buddy);
    if (!contact)
        contact = preferredContact(buddy);

    return contact;
}

ContactSet BuddyPreferredManager::preferredContacts(const BuddySet &buddies)
{
    if (buddies.isEmpty())
        return ContactSet();

    Contact contact = preferredContact2(*buddies.constBegin());

    Account account = contact.contactAccount();
    if (account.isNull())
        return ContactSet();

    Account commonAccount = getCommonAccount(buddies);
    if (!commonAccount)
        return ContactSet();

    ContactSet contacts;
    for (auto const &buddy : buddies)
        contacts.insert(preferredContact(buddy, commonAccount));

    return contacts;
}

Contact BuddyPreferredManager::preferredContactByPriority(const Buddy &buddy, const Account &account)
{
    if (account.isNull())
        return buddy.contacts().at(0);

    for (auto const &contact : buddy.contacts())
        if (contact.contactAccount() == account)
            return contact;

    return Contact::null;
}

Contact BuddyPreferredManager::preferredContact(const Buddy &buddy, bool includechats)
{
    return BuddyPreferredManager::preferredContact(buddy, Account::null, includechats);
}

Account BuddyPreferredManager::preferredAccount(const Buddy &buddy, bool includechats)
{
    Contact contact = BuddyPreferredManager::preferredContact(buddy, includechats);
    return contact.contactAccount();
}

Contact BuddyPreferredManager::preferredContactByUnreadMessages(const Buddy &buddy, const Account &account)
{
    Contact result;
    for (auto const &contact : buddy.contacts())
    {
        if (contact.unreadMessagesCount() > 0)
            result = morePreferredContactByStatus(result, contact, account);
    }
    return result;
}

Contact BuddyPreferredManager::preferredContactByChatWidgets(const Buddy &buddy, const Account &account)
{
    if (!m_chatWidgetRepository)
        return Contact::null;

    Contact result;
    for (auto chatwidget : m_chatWidgetRepository.data())
    {
        Chat chat = chatwidget->chat();
        if (chat.contacts().isEmpty())
            continue;
        Contact contact = *chat.contacts().constBegin();
        if (contact.ownerBuddy() != buddy)
            continue;
        result = morePreferredContactByStatus(result, contact, account);
    }
    return result;
}

Contact BuddyPreferredManager::preferredContactByStatus(const Buddy &buddy, const Account &account)
{
    Contact result;
    for (auto const &contact : buddy.contacts())
        result = morePreferredContactByStatus(result, contact, account);
    return result;
}

Contact
BuddyPreferredManager::morePreferredContactByStatus(const Contact &c1, const Contact &c2, const Account &account)
{
    if (!c1 || (account && c1.contactAccount() != account))
        return c2;

    if (!c2 || (account && c2.contactAccount() != account))
        return c1;

    if (c1.contactAccount().statusContainer()->status().isDisconnected() &&
        !c2.contactAccount().statusContainer()->status().isDisconnected())
        return c2;

    if (c2.contactAccount().statusContainer()->status().isDisconnected() &&
        !c1.contactAccount().statusContainer()->status().isDisconnected())
        return c1;

    return Contact::contactWithHigherStatus(c1, c2);
}

bool BuddyPreferredManager::isAccountCommon(const Account &account, const BuddySet &buddies)
{
    for (auto const &buddy : buddies)
        if (buddy.contacts(account).isEmpty())
            return false;

    return true;
}

Account BuddyPreferredManager::getCommonAccount(const BuddySet &buddies)
{
    for (auto const &account : m_accountManager->items())
        if (account.protocolHandler() && isAccountCommon(account, buddies))
            return account;

    return Account::null;
}
