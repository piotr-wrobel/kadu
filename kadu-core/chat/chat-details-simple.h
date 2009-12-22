/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CHAT_DETAILS_SIMPLE_H
#define CHAT_DETAILS_SIMPLE_H

#include "contacts/contact.h"

#include "chat/chat-details.h"

class ChatDetailsSimple : public ChatDetails
{
	Q_OBJECT

	Contact CurrentContact;

protected:
	virtual void load();

public:
	explicit ChatDetailsSimple(ChatShared *chatData);
	virtual ~ChatDetailsSimple();

	virtual void store();
	virtual bool shouldStore();

	virtual ChatType * type() const;
	virtual ContactSet contacts() const;
	virtual QString name() const;

	void setContact(Contact contact);
	Contact contact();
	void chatData();

};

#endif // CHAT_DETAILS_SIMPLE_H
