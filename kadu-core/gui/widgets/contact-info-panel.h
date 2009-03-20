/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTACT_INFO_PANEL_H
#define CONTACT_INFO_PANEL_H

#include "contacts/contact.h"
#include "configuration_aware_object.h"

#include "kadu_text_browser.h"

class ContactInfoPanel : public KaduTextBrowser, private ConfigurationAwareObject
{
	Q_OBJECT

	Contact CurrentContact;
	QString Template;
	QString Syntax;

protected:
	virtual void configurationUpdated();

public:
	explicit ContactInfoPanel(QWidget *parent = 0);
	virtual ~ContactInfoPanel();

public slots:
	void displayContact(Contact contact);

};

#endif // CONTACT_INFO_PANEL_H
