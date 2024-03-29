/*
 * %kadu copyright begin%
 * Copyright 2016 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "buddy-dummy-factory.h"
#include "moc_buddy-dummy-factory.cpp"

#include "accounts/account-storage.h"
#include "accounts/account.h"
#include "buddies/buddy-storage.h"
#include "buddies/buddy.h"
#include "contacts/contact-storage.h"
#include "contacts/contact.h"
#include "core/injected-factory.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "identities/identity-storage.h"
#include "identities/identity.h"

#include <QtWidgets/QApplication>

BuddyDummyFactory::BuddyDummyFactory(QObject *parent) : QObject{parent}
{
}

BuddyDummyFactory::~BuddyDummyFactory()
{
}

void BuddyDummyFactory::setAccountStorage(AccountStorage *accountStorage)
{
    m_accountStorage = accountStorage;
}

void BuddyDummyFactory::setBuddyStorage(BuddyStorage *buddyStorage)
{
    m_buddyStorage = buddyStorage;
}

void BuddyDummyFactory::setContactStorage(ContactStorage *contactStorage)
{
    m_contactStorage = contactStorage;
}

void BuddyDummyFactory::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void BuddyDummyFactory::setIdentityStorage(IdentityStorage *identityStorage)
{
    m_identityStorage = identityStorage;
}

void BuddyDummyFactory::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

Buddy BuddyDummyFactory::dummy()
{
    auto example = m_buddyStorage->create();
    example.setFirstName("Mark");
    example.setLastName("Smith");
    example.setNickName("Jimbo");
    example.setDisplay("Jimbo");
    example.setMobile("+48123456789");
    example.setEmail("jimbo@mail.server.net");
    example.setHomePhone("+481234567890");

    auto identity = m_identityStorage->create();
    identity.setName(QApplication::translate("Buddy", "Example identity"));

    auto account = m_accountStorage->create("");
    account.setAccountIdentity(identity);

    auto contact = m_contactStorage->create();
    contact.setContactAccount(account);
    contact.setOwnerBuddy(example);
    contact.setId("999999");
    contact.setCurrentStatus(Status(StatusType::Away, QApplication::translate("Buddy", "Example description")));

    example.addContact(contact);
    example.setAnonymous(false);

    return example;
}
