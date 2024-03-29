/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "buddy-manager.h"
#include "moc_buddy-manager.cpp"

#include "accounts/account.h"
#include "buddies/buddy-list.h"
#include "buddies/buddy-storage.h"
#include "configuration/configuration-api.h"
#include "configuration/configuration-manager.h"
#include "configuration/configuration.h"
#include "contacts/contact-manager.h"
#include "contacts/contact.h"
#include "core/core.h"
#include "roster/roster.h"
#include "storage/storage-point.h"

BuddyManager::BuddyManager(QObject *parent) : Manager<Buddy>{parent}
{
}

BuddyManager::~BuddyManager()
{
}

void BuddyManager::setBuddyStorage(BuddyStorage *buddyStorage)
{
    m_buddyStorage = buddyStorage;
}

void BuddyManager::setConfigurationManager(ConfigurationManager *configurationManager)
{
    m_configurationManager = configurationManager;
}

void BuddyManager::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void BuddyManager::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

void BuddyManager::init()
{
    m_configurationManager->registerStorableObject(this);
}

void BuddyManager::done()
{
    m_configurationManager->unregisterStorableObject(this);
}

void BuddyManager::load()
{
    QMutexLocker locker(&mutex());

    Manager<Buddy>::load();
}

Buddy BuddyManager::loadStubFromStorage(const std::shared_ptr<StoragePoint> &storagePoint)
{
    return m_buddyStorage->loadStubFromStorage(storagePoint);
}

void BuddyManager::itemAboutToBeAdded(Buddy buddy)
{
    QMutexLocker locker(&mutex());

    connect(buddy, SIGNAL(updated()), this, SLOT(buddyDataUpdated()));
    connect(buddy, SIGNAL(buddySubscriptionChanged()), this, SLOT(buddySubscriptionChanged()));

    connect(buddy, SIGNAL(contactAboutToBeAdded(Contact)), this, SLOT(buddyContactAboutToBeAdded(Contact)));
    connect(buddy, SIGNAL(contactAdded(Contact)), this, SLOT(buddyContactAdded(Contact)));
    connect(buddy, SIGNAL(contactAboutToBeRemoved(Contact)), this, SLOT(buddyContactAboutToBeRemoved(Contact)));
    connect(buddy, SIGNAL(contactRemoved(Contact)), this, SLOT(buddyContactRemoved(Contact)));

    emit buddyAboutToBeAdded(buddy);
}

void BuddyManager::itemAdded(Buddy buddy)
{
    emit buddyAdded(buddy);
}

void BuddyManager::itemAboutToBeRemoved(Buddy buddy)
{
    for (auto const &contact : buddy.contacts())
        contact.setOwnerBuddy(Buddy::null);

    emit buddyAboutToBeRemoved(buddy);
}

void BuddyManager::itemRemoved(Buddy buddy)
{
    QMutexLocker locker(&mutex());

    disconnect(buddy, 0, this, 0);

    emit buddyRemoved(buddy);
}

QString BuddyManager::mergeValue(const QString &destination, const QString &source) const
{
    if (destination.isEmpty())
        return source;
    else
        return destination;
}

void BuddyManager::mergeBuddies(Buddy destination, Buddy source)
{
    QMutexLocker locker(&mutex());

    if (destination == source)
        return;

    ensureLoaded();

    destination.setEmail(mergeValue(destination.email(), source.email()));
    destination.setHomePhone(mergeValue(destination.homePhone(), source.homePhone()));
    destination.setMobile(mergeValue(destination.mobile(), source.mobile()));
    destination.setWebsite(mergeValue(destination.website(), source.website()));

    // we need to move contacts before removing source buddy as this would cause
    // these contacts to detach and remove from roster
    // i think this is another reason why we should not automate too much
    // we should just manually delete all contacts when buddy is removed

    for (auto const &contact : source.contacts())
        contact.setOwnerBuddy(destination);

    removeItem(source);

    source.setAnonymous(true);
    // each item that stores pointer to "source" will now use the same uuid as "destination"
    source.data()->setUuid(destination.uuid());

    m_configurationManager->flush();
}

Buddy BuddyManager::byDisplay(const QString &display, NotFoundAction action)
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    if (display.isEmpty())
        return Buddy::null;

    for (auto &buddy : items())
    {
        if (display == buddy.display())
            return buddy;
    }

    if (ActionReturnNull == action)
        return Buddy::null;

    auto buddy = m_buddyStorage->create();
    buddy.setDisplay(display);

    if (ActionCreateAndAdd == action)
        addItem(buddy);

    return buddy;
}

Buddy BuddyManager::byId(Account account, const QString &id, NotFoundAction action)
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    auto contact = m_contactManager->byId(account, id, action);
    if (contact.isNull())
        return Buddy::null;

    return byContact(contact, action);
}

Buddy BuddyManager::byContact(Contact contact, NotFoundAction action)
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    if (!contact)
        return Buddy::null;

    if (ActionReturnNull == action || !contact.isAnonymous())
        return contact.ownerBuddy();

    if (!contact.ownerBuddy())
        contact.setOwnerBuddy(m_buddyStorage->create());

    if (ActionCreateAndAdd == action)
        addItem(contact.ownerBuddy());

    return contact.ownerBuddy();
}

Buddy BuddyManager::byUuid(const QUuid &uuid)
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    if (uuid.isNull())
        return m_buddyStorage->create();

    for (auto const &buddy : items())
        if (buddy.uuid() == uuid)
            return buddy;

    return m_buddyStorage->create();
}

void BuddyManager::removeBuddyIfEmpty(Buddy buddy, bool checkOnlyForContacts)
{
    if (!buddy)
        return;

    if (buddy.isEmpty(checkOnlyForContacts))
        removeItem(buddy);
}

void BuddyManager::clearOwnerAndRemoveEmptyBuddy(Contact contact, bool checkBuddyOnlyForOtherContacts)
{
    if (!contact)
        return;

    Buddy owner = contact.ownerBuddy();
    contact.setOwnerBuddy(Buddy::null);
    removeBuddyIfEmpty(owner, checkBuddyOnlyForOtherContacts);
}

BuddyList BuddyManager::buddies(Account account, bool includeAnonymous)
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    BuddyList result;

    for (auto const &buddy : items())
        if (buddy.hasContact(account) && (includeAnonymous || !buddy.isAnonymous()))
            result << buddy;

    return result;
}

void BuddyManager::buddyDataUpdated()
{
    QMutexLocker locker(&mutex());

    Buddy buddy(sender());
    if (!buddy.isNull())
        emit buddyUpdated(buddy);
}

void BuddyManager::buddySubscriptionChanged()
{
    QMutexLocker locker(&mutex());

    Buddy buddy(sender());
    if (!buddy.isNull())
        emit buddySubscriptionChanged(buddy);
}

void BuddyManager::buddyContactAboutToBeAdded(const Contact &contact)
{
    QMutexLocker locker(&mutex());

    Buddy buddy(sender());
    if (!buddy.isNull())
        emit buddyContactAboutToBeAdded(buddy, contact);
}

void BuddyManager::buddyContactAdded(const Contact &contact)
{
    QMutexLocker locker(&mutex());

    Buddy buddy(sender());
    if (!buddy.isNull())
        emit buddyContactAdded(buddy, contact);
}

void BuddyManager::buddyContactAboutToBeRemoved(const Contact &contact)
{
    QMutexLocker locker(&mutex());

    Buddy buddy(sender());
    if (!buddy.isNull())
        emit buddyContactAboutToBeRemoved(buddy, contact);
}

void BuddyManager::buddyContactRemoved(const Contact &contact)
{
    QMutexLocker locker(&mutex());

    Buddy buddy(sender());
    if (!buddy.isNull())
        emit buddyContactRemoved(buddy, contact);
}
