/*
 * %kadu copyright begin%
 * Copyright 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtSql/QSqlQuery>

#include "accounts/account.h"
#include "contacts/contact-manager.h"
#include "contacts/contact.h"
#include "core/core.h"

#include "storage/sql-accounts-mapping.h"

#include "sql-contacts-mapping.h"
#include "moc_sql-contacts-mapping.cpp"

SqlContactsMapping::SqlContactsMapping(
    const QSqlDatabase &database, SqlAccountsMapping *accountsMapping, QObject *parent)
        : QObject(parent), Database(database), Mutex(QMutex::Recursive), AccountsMapping(accountsMapping)
{
    Q_ASSERT(AccountsMapping);
}

SqlContactsMapping::~SqlContactsMapping()
{
}

void SqlContactsMapping::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

void SqlContactsMapping::init()
{
    loadMappingsFromDatabase();

    connect(m_contactManager, SIGNAL(contactUpdated(Contact)), this, SLOT(contactUpdated(Contact)));
}

void SqlContactsMapping::contactUpdated(const Contact &contact)
{
    QMutexLocker locker(&Mutex);

    int id = idByContact(contact, false);
    // not all contacts are mapped
    if (id <= 0)
        return;

    QSqlQuery query(Database);
    query.prepare("UPDATE kadu_contacts SET account_id = :account_id, contact = :contact WHERE id = :id");
    query.bindValue(":account_id", SqlAccountsMapping::idByAccount(contact.contactAccount()));
    query.bindValue(":contact", contact.id());
    query.bindValue(":id", id);
    query.exec();
}

void SqlContactsMapping::addMapping(int id, const Contact &contact)
{
    QMutexLocker locker(&Mutex);

    contact.addProperty("sql_history:id", id, CustomProperties::NonStorable);
    ContactMapping.insert(id, contact);
}

void SqlContactsMapping::loadMappingsFromDatabase()
{
    QMutexLocker locker(&Mutex);

    QSqlQuery query(Database);
    query.prepare("SELECT id, account_id, contact FROM kadu_contacts");

    query.setForwardOnly(true);
    query.exec();

    while (query.next())
    {
        int id = query.value(0).toInt();
        Account account = AccountsMapping->accountById(query.value(1).toInt());
        QString contactId = query.value(2).toString();

        if (id <= 0)
            continue;

        // This contact needs to be known to the manager even if it's not on our roster,
        // in case we want to add him later or even talk to her without adding.
        Contact contact = m_contactManager->byId(account, contactId, ActionCreateAndAdd);
        if (contact)
            addMapping(id, contact);
    }
}

Contact SqlContactsMapping::contactById(int sqlId) const
{
    QMutexLocker locker(&Mutex);

    if (ContactMapping.contains(sqlId))
        return ContactMapping.value(sqlId);
    else
        return Contact::null;
}

int SqlContactsMapping::idByContact(const Contact &contact, bool create)
{
    QMutexLocker locker(&Mutex);

    int id = contact.property("sql_history:id", 0).toInt();
    if (!create || id > 0)
        return id;

    QSqlQuery query(Database);
    query.prepare("INSERT INTO kadu_contacts (account_id, contact) VALUES (:account_id, :contact)");
    query.bindValue(":account_id", SqlAccountsMapping::idByAccount(contact.contactAccount()));
    query.bindValue(":contact", contact.id());
    query.exec();

    id = query.lastInsertId().toInt();
    addMapping(id, contact);

    return id;
}

const QMap<int, Contact> &SqlContactsMapping::mapping() const
{
    QMutexLocker locker(&Mutex);

    return ContactMapping;
}
