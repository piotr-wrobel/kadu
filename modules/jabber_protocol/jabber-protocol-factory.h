/*
 * %kadu copyright begin%
 * Copyright 2009, 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2009, 2010 Piotr Galiszewski (piotrgaliszewski@gmail.com)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
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

#ifndef JABBER_PROTOCOL_FACTORY_H
#define JABBER_PROTOCOL_FACTORY_H

#include "protocols/protocol-factory.h"

class JabberProtocolFactory : public ProtocolFactory
{
	Q_DISABLE_COPY(JabberProtocolFactory)

	static JabberProtocolFactory *Instance;
	QList<StatusType *> SupportedStatusTypes;
	QRegExp IdRegularExpression;

	JabberProtocolFactory();

public:
	static JabberProtocolFactory * instance();

	virtual Protocol * createProtocolHandler(Account account);
	virtual AccountDetails * createAccountDetails(AccountShared *accountShared);
	virtual ContactDetails * createContactDetails(ContactShared *contactShared);
	virtual AccountAddWidget * newAddAccountWidget(QWidget *parent);
	virtual QWidget * newCreateAccountWidget(QWidget *parent);
	virtual AccountEditWidget* newEditAccountWidget(Account, QWidget*);
	virtual ContactWidget * newContactWidget(Contact contact, QWidget *parent = 0);
	virtual QList<StatusType *> supportedStatusTypes();
	virtual QString idLabel();
	virtual QRegExp idRegularExpression();

	virtual QString name() { return "jabber"; }
	virtual QString displayName() { return "Jabber"; }

	virtual QIcon icon();
	virtual QString iconPath();

};

#endif // JABBER_PROTOCOL_FACTORY_H
