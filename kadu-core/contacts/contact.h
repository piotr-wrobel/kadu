/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTACT_H
#define CONTACT_H

#include <QtNetwork/QHostAddress>
#include <QtXml/QDomElement>

#include "accounts/account.h"
#include "buddies/buddy.h"
#include "status/status.h"
#include "storage/shared-base.h"

#include "exports.h"

class Avatar;
class ContactDetails;
class ContactShared;
class XmlConfigFile;

class KADUAPI Contact : public SharedBase<ContactShared>
{

public:
	static Contact create();
	static Contact loadFromStorage(StoragePoint *storage);
	static Contact null;
	
	Contact();
	Contact(ContactShared *data);
	Contact(QObject *data);
	Contact(const Contact &copy);
	virtual ~Contact();

	Contact & operator = (const Contact &copy);

	virtual bool validateId();
	bool isValid();

	KaduSharedBase_Property(ContactDetails *, details, Details)
	KaduSharedBase_PropertyRead(QUuid, uuid, Uuid)
	KaduSharedBase_PropertyRead(StoragePoint *, storage, Storage)
	KaduSharedBase_Property(Account, contactAccount, ContactAccount)
	KaduSharedBase_Property(Avatar, contactAvatar, ContactAvatar)
	KaduSharedBase_Property(Buddy, ownerBuddy, OwnerBuddy)
	KaduSharedBase_Property(QString, id, Id)
	KaduSharedBase_Property(Status, currentStatus, CurrentStatus)
	KaduSharedBase_Property(QString, protocolVersion, ProtocolVersion)
	KaduSharedBase_Property(QHostAddress, address, Address)
	KaduSharedBase_Property(unsigned int, port, Port)
	KaduSharedBase_Property(QString, dnsName, DnsName)

};

Q_DECLARE_METATYPE(Contact)

#endif // CONTACT_H
