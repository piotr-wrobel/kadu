/*
 * %kadu copyright begin%
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010 Piotr Galiszewski (piotrgaliszewski@gmail.com)
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
#include "contacts/contact.h"
#include "core/core.h"
#include "identities/identity-manager.h"
#include "misc/misc.h"
#include "protocols/protocol.h"

#include "identity-shared.h"

IdentityShared * IdentityShared::loadFromStorage(StoragePoint *storagePoint)
{
	IdentityShared *identityShared = new IdentityShared();
	identityShared->setStorage(storagePoint);
	identityShared->load();

	return identityShared;
}

IdentityShared::IdentityShared(const QUuid &uuid) :
		BaseStatusContainer(this), Shared(uuid)
{
}

IdentityShared::~IdentityShared()
{
}

StorableObject * IdentityShared::storageParent()
{
	return IdentityManager::instance();
}

QString IdentityShared::storageNodeName()
{
	return QLatin1String("Identity");
}

void IdentityShared::load()
{
	if (!isValidStorage())
		return;

	Shared::load();

	Name = loadValue<QString>("Name");

	XmlConfigFile *configurationStorage = storage()->storage();

	QDomElement accountsNode = configurationStorage->getNode(storage()->point(), "Accounts", XmlConfigFile::ModeFind);
	if (!accountsNode.isNull())
	{
		QDomNodeList accountsList = accountsNode.elementsByTagName("Account");

		int count = accountsList.count();
		for (int i = 0; i < count; i++)
		{
			QDomElement accountElement = accountsList.at(i).toElement();
			if (!accountElement.isNull())
			{
				Account account = AccountManager::instance()->byUuid(accountElement.text());
				if (!account.isNull())
					Accounts << account;
			}
		}
	}
}

void IdentityShared::store()
{
	if (!isValidStorage())
		return;

	storeValue("Name", Name);

	XmlConfigFile *configurationStorage = storage()->storage();

	if (Accounts.count())
	{
		QDomElement accountsNode = configurationStorage->getNode(storage()->point(), "Accounts", XmlConfigFile::ModeCreate);
		foreach (const Account account, Accounts)
			configurationStorage->appendTextNode(accountsNode, "Account", account.uuid().toString());
	}
	else
		configurationStorage->removeNode(storage()->point(), "Accounts");
}

void IdentityShared::aboutToBeRemoved()
{
	Accounts = QList<Account>();
}

void IdentityShared::addAccount(Account account)
{
	Accounts.append(account);
}

void IdentityShared::removeAccount(Account account)
{
	Accounts.removeAll(account);
}

bool IdentityShared::hasAccount(Account account)
{
	return Accounts.contains(account);
}

void IdentityShared::setStatus(Status status)
{
	foreach (Account account, Accounts)
		if (account.statusContainer())
			account.statusContainer()->setStatus(status);
}

Status IdentityShared::status()
{
	foreach (Account account, Accounts)
		if (account.statusContainer())
			return Accounts[0].statusContainer()->status();
	return Status::null;
}

QString IdentityShared::statusName()
{
	return Status::name(status(), false);
}

QPixmap IdentityShared::statusPixmap()
{
	return statusPixmap(status().type());
}

QPixmap IdentityShared::statusPixmap(Status status)
{
	return statusPixmap(status.type());
}

QPixmap IdentityShared::statusPixmap(const QString &statusType)
{
	foreach (Account account, Accounts)
		if (account.statusContainer())
			return account.statusContainer()->statusPixmap(statusType);
	return QPixmap();
}

QList<StatusType *> IdentityShared::supportedStatusTypes()
{
	foreach (Account account, Accounts)
		if (account.statusContainer())
			return account.statusContainer()->supportedStatusTypes();
	return QList<StatusType *>();
}

int IdentityShared::maxDescriptionLength()
{
	foreach (Account account, Accounts)
		if (account.statusContainer())
			return account.statusContainer()->maxDescriptionLength();
	return -1;
}

void IdentityShared::setPrivateStatus(bool isPrivate)
{
	foreach (Account account, Accounts)
		if (account.statusContainer())
			account.statusContainer()->setPrivateStatus(isPrivate);
}
