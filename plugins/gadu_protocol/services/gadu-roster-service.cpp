/*
 * %kadu copyright begin%
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "gadu-roster-service.h"
#include "moc_gadu-roster-service.cpp"

#include "gadu-account-data.h"
#include "helpers/gadu-list-helper.h"
#include "server/gadu-connection.h"
#include "server/gadu-writable-session-token.h"
#include "services/gadu-roster-state-machine.h"

#include "buddies/buddy-manager.h"
#include "roster/roster-entry-state.h"
#include "roster/roster-entry.h"
#include "roster/roster-notifier.h"
#include "roster/roster-replacer.h"

#include <QtCore/QScopedArrayPointer>
#include <libgadu.h>

GaduRosterService::GaduRosterService(
    GaduListHelper *gaduListHelper, const QVector<Contact> &contacts, Protocol *protocol)
        : RosterService{contacts, protocol}, m_stateMachine{new GaduRosterStateMachine{this, protocol}},
          m_gaduListHelper{gaduListHelper}
{
    connect(this, SIGNAL(contactAdded(Contact)), this, SLOT(rosterChanged()));
    connect(this, SIGNAL(contactRemoved(Contact)), this, SLOT(rosterChanged()));
    connect(this, SIGNAL(contactUpdatedLocally(Contact)), this, SLOT(rosterChanged()));

    connect(m_stateMachine, SIGNAL(performGet()), SLOT(importContactList()));
    connect(m_stateMachine, SIGNAL(performPut()), SLOT(exportContactList()));
}

GaduRosterService::~GaduRosterService()
{
}

void GaduRosterService::setBuddyManager(BuddyManager *buddyManager)
{
    m_buddyManager = buddyManager;
}

void GaduRosterService::init()
{
    m_stateMachine->start();
}

void GaduRosterService::setConnection(GaduConnection *connection)
{
    m_connection = connection;
}

void GaduRosterService::setRosterNotifier(RosterNotifier *rosterNotifier)
{
    m_rosterNotifier = rosterNotifier;
}

void GaduRosterService::setRosterReplacer(RosterReplacer *rosterReplacer)
{
    m_rosterReplacer = rosterReplacer;
}

void GaduRosterService::prepareRoster()
{
    auto requiresSynchronization = false;
    for (auto &&contact : contacts())
        if (contact.rosterEntry())
        {
            if (contact.rosterEntry()->state() == RosterEntryState::Detached)   // GG does not support detached contacts
                contact.rosterEntry()->setHasLocalChanges();
            else
                contact.rosterEntry()->fixupInitialState();
            requiresSynchronization |= contact.rosterEntry()->requiresSynchronization();
        }
    if (requiresSynchronization)
        rosterChanged();
}

void GaduRosterService::putFinished(bool ok)
{
    if (ok)
    {
        emit stateMachinePutFinished();
        if (m_rosterNotifier)
            m_rosterNotifier.data()->notifyExportSucceeded(account());
    }
    else
    {
        emit stateMachinePutFailed();
        if (m_rosterNotifier)
            m_rosterNotifier.data()->notifyExportFailed(account());
    }
}

void GaduRosterService::getFinished(bool ok)
{
    if (ok)
    {
        emit stateMachineGetFinished();
        if (m_rosterNotifier)
            m_rosterNotifier.data()->notifyImportSucceeded(account());
    }
    else
    {
        emit stateMachineGetFailed();
        if (m_rosterNotifier)
            m_rosterNotifier.data()->notifyImportFailed(account());
    }
}

void GaduRosterService::handleEventUserlist100GetReply(struct gg_event *e)
{
    if (!m_stateMachine->isPerformingGet())
    {
        return;
    }

    auto accountData = GaduAccountData{account()};
    if (e->event.userlist100_reply.format_type != GG_USERLIST100_FORMAT_TYPE_GG70)
    {
        getFinished(false);
        return;
    }

    auto content = e->event.userlist100_reply.reply;
    if (!content)
    {
        getFinished(false);
        return;
    }

    if (accountData.userlistVersion() != (int)e->event.userlist100_reply.version)
    {
        auto content2 = QByteArray{content};
        auto buddies = m_gaduListHelper->byteArrayToBuddyList(account(), content2);
        getFinished(true);

        auto result = m_rosterReplacer->replaceRoster(account(), buddies, haveToAskForAddingContacts());
        accountData.setUserlistVersion(e->event.userlist100_reply.version);
        accountData.setInitialRosterImport(false);

        for (auto &&contact : result.first)
            contact.rosterEntry()->setSynchronized();

        for (auto &&contact : result.second)
        {
            auto ownerBuddy = contact.ownerBuddy();
            contact.setOwnerBuddy(Buddy::null);
            m_buddyManager->removeBuddyIfEmpty(ownerBuddy, true);
            removeContact(contact);
            contact.rosterEntry()->setSynchronized();
        }

        // cleanup references, so buddy and contact instances can be removed
        // this is really a hack, we need to call aboutToBeRemoved someway for non-manager contacts and buddies too
        // or just only store managed only, i dont know yet
        for (auto &&buddy : buddies)
        {
            for (auto &&contact : buddy.contacts())
                contact.data()->aboutToBeRemoved();
            buddy.data()->aboutToBeRemoved();
        }
    }
    else
    {
        markSynchronizingAsSynchronized();
        getFinished(true);
    }
}

void GaduRosterService::handleEventUserlist100PutReply(struct gg_event *e)
{
    if (!m_stateMachine->isPerformingPut())
        return;

    if (e->event.userlist100_reply.type == GG_USERLIST100_REPLY_ACK)
    {
        auto accountData = GaduAccountData{account()};
        accountData.setUserlistVersion(e->event.userlist100_reply.version);
        markSynchronizingAsSynchronized();
        putFinished(true);
        return;
    }

    putFinished(false);
}

void GaduRosterService::handleEventUserlist100Reply(struct gg_event *e)
{
    switch (e->event.userlist100_reply.type)
    {
    case GG_USERLIST100_REPLY_LIST:
        handleEventUserlist100GetReply(e);
        break;
    case GG_USERLIST100_REPLY_ACK:
    case GG_USERLIST100_REPLY_REJECT:
        handleEventUserlist100PutReply(e);
        break;
    }
}

void GaduRosterService::handleEventUserlist100Version(gg_event *e)
{
    auto accountData = GaduAccountData{account()};
    if (accountData.userlistVersion() != (int)e->event.userlist100_version.version)
        emit stateMachineRemoteDirty();
}

void GaduRosterService::rosterChanged()
{
    QMetaObject::invokeMethod(this, "stateMachineLocalDirty", Qt::QueuedConnection);
}

void GaduRosterService::markSynchronizingAsSynchronized()
{
    for (auto &&contact : m_synchronizingContacts)
        contact.rosterEntry()->setSynchronized();
}

bool GaduRosterService::haveToAskForAddingContacts() const
{
    auto accountData = GaduAccountData{account()};

    // if already synchronized, never ask
    if (-1 != accountData.userlistVersion())
        return false;

    // if not yet synchronized but also not migrating from 0.9.x, i.e., it's a clean install, do not ask as well
    if (accountData.initialRosterImport())
        return false;

    // here is the case for migrating from 0.9.x - ask then
    return true;
}

void GaduRosterService::importContactList()
{
    if (!m_connection || !m_connection.data()->hasSession())
    {
        emit getFinished(false);
        return;
    }

    m_synchronizingContacts = contacts();
    for (auto &&contact : m_synchronizingContacts)
        contact.rosterEntry()->setSynchronizingFromRemote();

    auto writableSessionToken = m_connection.data()->writableSessionToken();
    int ret = gg_userlist100_request(
        writableSessionToken.rawSession(), GG_USERLIST100_GET, 0, GG_USERLIST100_FORMAT_TYPE_GG70, 0);
    if (-1 == ret)
    {
        markSynchronizingAsSynchronized();
        emit getFinished(false);
    }
}

void GaduRosterService::exportContactList()
{
    if (!m_connection || !m_connection.data()->hasSession())
    {
        putFinished(false);
        return;
    }

    m_synchronizingContacts = contacts();
    for (auto &&contact : m_synchronizingContacts)
        contact.rosterEntry()->setSynchronizingToRemote();

    auto contacts = m_gaduListHelper->contactListToByteArray(m_synchronizingContacts);
    auto accountData = GaduAccountData{account()};
    auto writableSessionToken = m_connection.data()->writableSessionToken();
    auto ret = gg_userlist100_request(
        writableSessionToken.rawSession(), GG_USERLIST100_PUT, accountData.userlistVersion(),
        GG_USERLIST100_FORMAT_TYPE_GG70, contacts.constData());
    if (-1 == ret)
    {
        markSynchronizingAsSynchronized();
        putFinished(false);
    }
}
