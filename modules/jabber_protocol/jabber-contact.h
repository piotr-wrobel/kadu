/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBER_CONTACT_H
#define JABBER_CONTACT_H

#include <xmpp.h>

#include "contacts/contact.h"

class Account;

class JabberContact : public Contact
{
	unsigned long MaxImageSize;

public:
	JabberContact(Account account, Buddy buddy, const QString &id, bool loaded = false) :
			Contact(account, buddy, id, loaded) {}
	JabberContact(Account account, Buddy buddy, const QString &id, StoragePoint *storage) :
			Contact(account, buddy, id, storage) {}
	JabberContact(StoragePoint *storage) :
			Contact(storage) {}

	virtual bool validateId();

	void setMaxImageSize(unsigned long maxImageSize) { MaxImageSize = maxImageSize; }

};

#endif // JABBER_CONTACT_H
