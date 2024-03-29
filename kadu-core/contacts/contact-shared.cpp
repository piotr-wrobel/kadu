/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "accounts/account-manager.h"
#include "accounts/account.h"
#include "buddies/buddy-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "contacts/contact-manager.h"
#include "core/injected-factory.h"
#include "misc/change-notifier.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol.h"
#include "protocols/protocols-manager.h"
#include "roster/roster-entry-state.h"
#include "roster/roster-entry.h"

#include "contact-shared.h"
#include "moc_contact-shared.cpp"

ContactShared::ContactShared(const QUuid &uuid)
        : Shared(uuid), Priority(-1), MaximumImageSize(0), UnreadMessagesCount(0), Blocking(false),
          IgnoreNextStatusChange(false)
{
}

ContactShared::~ContactShared()
{
    ref.ref();

    disconnect(m_protocolsManager, 0, this, 0);

    protocolFactoryUnregistered(m_protocolsManager->byName(ContactAccount->protocolName()));

    delete OwnerBuddy;
    delete ContactAccount;
}

void ContactShared::setAccountManager(AccountManager *accountManager)
{
    m_accountManager = accountManager;
}

void ContactShared::setBuddyManager(BuddyManager *buddyManager)
{
    m_buddyManager = buddyManager;
}

void ContactShared::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void ContactShared::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

void ContactShared::setProtocolsManager(ProtocolsManager *protocolsManager)
{
    m_protocolsManager = protocolsManager;
}

void ContactShared::init()
{
    Entry = new RosterEntry(this);
    connect(&Entry->hasLocalChangesNotifier(), SIGNAL(changed()), this, SIGNAL(updatedLocally()));

    ContactAccount = new Account();
    OwnerBuddy = new Buddy();

    connect(
        m_protocolsManager, SIGNAL(protocolFactoryRegistered(ProtocolFactory *)), this,
        SLOT(protocolFactoryRegistered(ProtocolFactory *)));
    connect(
        m_protocolsManager, SIGNAL(protocolFactoryUnregistered(ProtocolFactory *)), this,
        SLOT(protocolFactoryUnregistered(ProtocolFactory *)));

    connect(&changeNotifier(), SIGNAL(changed()), this, SLOT(changeNotifierChanged()));
}

StorableObject *ContactShared::storageParent()
{
    return m_contactManager;
}

QString ContactShared::storageNodeName()
{
    return QStringLiteral("Contact");
}

void ContactShared::load()
{
    if (!isValidStorage())
        return;

    Shared::load();

    Id = loadValue<QString>("Id");
    Priority = loadValue<int>("Priority", -1);

    // TODO: remove after 01.05.2015
    // It's an explicit hack for update path from 0.10.1-0.11.x to 0.12+. 0.10/0.11 didn't
    // have Detached property. But they did have an explicit hack for totally ignoring
    // what Facebook says about groups, thus allowing users to place their Facebook contacts
    // in groups in Kadu. And without below hack all this group information is overriden
    // by useless a Facebook-provided group until we try to upload something to roster
    // for the first time, we fail and only then we set Detached to true, when group
    // information is already lost.
    bool detached =
        hasValue("Detached") ? loadValue<bool>("Detached", false) : Id.endsWith(QStringLiteral("@chat.facebook.com"));
    bool dirty = loadValue<bool>("Dirty", true);
    if (detached)
        Entry->setDetached();
    else if (dirty)
        Entry->setHasLocalChanges();
    else
        Entry->setSynchronized();

    *ContactAccount = m_accountManager->byUuid(loadValue<QString>("Account"));
    doSetOwnerBuddy(m_buddyManager->byUuid(loadValue<QString>("Buddy")));

    protocolFactoryRegistered(m_protocolsManager->byName(ContactAccount->protocolName()));
    addToBuddy();
}

void ContactShared::aboutToBeRemoved()
{
    // clean up references
    removeFromBuddy();
    doSetOwnerBuddy(Buddy::null);

    changeNotifier().notify();
}

void ContactShared::store()
{
    if (!isValidStorage())
        return;

    ensureLoaded();

    Shared::store();

    storeValue("Id", Id);
    storeValue("Priority", Priority);

    storeValue("Dirty", RosterEntryState::Synchronized != Entry->state());
    // Detached property needs to be always stored, see the load() method.
    storeValue("Detached", RosterEntryState::Detached == Entry->state());

    storeValue("Account", ContactAccount->uuid().toString());
    storeValue("Buddy", !isAnonymous() ? OwnerBuddy->uuid().toString() : QString());
    removeValue("Avatar");
    removeValue("Contact");
}

bool ContactShared::shouldStore()
{
    ensureLoaded();

    if (!UuidStorableObject::shouldStore())
        return false;

    if (Id.isEmpty() || ContactAccount->uuid().isNull())
        return false;

    // we dont need data for non-roster contacts only from 4 version of sql schema
    if (m_configuration->deprecatedApi()->readNumEntry("History", "Schema", 0) < 4)
        return true;

    return !isAnonymous() || rosterEntry()->requiresSynchronization() || customProperties()->shouldStore();
}

void ContactShared::addToBuddy()
{
    // dont add to buddy if details are not available
    if (*OwnerBuddy)
        OwnerBuddy->addContact(this);
}

void ContactShared::removeFromBuddy()
{
    if (*OwnerBuddy)
        OwnerBuddy->removeContact(this);
}

void ContactShared::setOwnerBuddy(const Buddy &buddy)
{
    ensureLoaded();

    if (*OwnerBuddy == buddy)
        return;

    /* NOTE: This guard is needed to avoid deleting this object when removing
     * Contact from Buddy which may hold last reference to it and thus wants to
     * delete it. But we don't want this to happen.
     */
    Contact guard(this);

    removeFromBuddy();
    doSetOwnerBuddy(buddy);
    addToBuddy();

    Entry->setHasLocalChanges();
    changeNotifier().notify();

    emit buddyUpdated();
}

void ContactShared::setContactAccount(const Account &account)
{
    ensureLoaded();

    if (*ContactAccount == account)
        return;

    if (*ContactAccount && ContactAccount->protocolHandler() && ContactAccount->protocolHandler()->protocolFactory())
        protocolFactoryUnregistered(ContactAccount->protocolHandler()->protocolFactory());

    *ContactAccount = account;

    if (*ContactAccount && ContactAccount->protocolHandler() && ContactAccount->protocolHandler()->protocolFactory())
        protocolFactoryRegistered(ContactAccount->protocolHandler()->protocolFactory());

    changeNotifier().notify();
}

void ContactShared::protocolFactoryRegistered(ProtocolFactory *protocolFactory)
{
    ensureLoaded();

    if (!protocolFactory || !*ContactAccount || ContactAccount->protocolName() != protocolFactory->name())
        return;

    changeNotifier().notify();
}

void ContactShared::protocolFactoryUnregistered(ProtocolFactory *protocolFactory)
{
    ensureLoaded();

    if (!protocolFactory || ContactAccount->protocolName() != protocolFactory->name())
        return;

    /* NOTE: This guard is needed to avoid deleting this object when detaching
     * Contact from Buddy which may hold last reference to it and thus wants to
     * delete it. But we don't want this to happen.
     */
    Contact guard(this);
    changeNotifier().notify();
}

void ContactShared::setId(const QString &id)
{
    ensureLoaded();

    if (Id == id)
        return;

    QString oldId = Id;
    Id = id;

    changeNotifier().notify();
}

RosterEntry *ContactShared::rosterEntry()
{
    ensureLoaded();

    return Entry;
}

/*
 * @todo: move this comment somewhere
 *
 * Sets state if this contact to \p dirty. All contacts are dirty by default.
 *
 * Dirty contacts with anonymous owner buddies are considered dirty removed and will
 * never be added to roster as long as this state lasts and will in effect be removed
 * from remote roster. Dirty contacts with not anonymous owner buddies are considered
 * dirty added and will always be added to roster, even if remote roster marked
 * them as removed.
 *
 * When adding contacts with anononymous owner buddies to the manager, always make sure
 * to mark them not dirty, otherwise they will be considered dirty removed and will
 * not be added to roster if remote roster says so, which is probably not what one expects.
 */

void ContactShared::changeNotifierChanged()
{
    emit updated();
}

void ContactShared::doSetOwnerBuddy(const Buddy &buddy)
{
    if (*OwnerBuddy)
        disconnect(*OwnerBuddy, 0, this, 0);

    *OwnerBuddy = buddy;

    if (*OwnerBuddy)
        connect(*OwnerBuddy, SIGNAL(updated()), this, SIGNAL(buddyUpdated()));
}

void ContactShared::setPriority(int priority)
{
    ensureLoaded();
    if (Priority != priority)
    {
        Priority = priority;
        changeNotifier().notify();
        emit priorityUpdated();
    }
}

bool ContactShared::isAnonymous()
{
    ensureLoaded();

    if (!OwnerBuddy)
        return true;

    if (!(*OwnerBuddy))
        return true;

    return OwnerBuddy->isAnonymous();
}

QString ContactShared::display(bool useBuddyData)
{
    ensureLoaded();

    if (!useBuddyData || !OwnerBuddy || !(*OwnerBuddy) || OwnerBuddy->display().isEmpty())
        return Id;

    return OwnerBuddy->display();
}

KaduShared_PropertyPtrReadDef(ContactShared, Account, contactAccount, ContactAccount)
    KaduShared_PropertyPtrReadDef(ContactShared, Buddy, ownerBuddy, OwnerBuddy)
