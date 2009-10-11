/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTACT_DATA_WINDOW_H
#define CONTACT_DATA_WINDOW_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QWidget>

#include "contacts/contact.h"
#include "gui/widgets/contact-general-configuration-widget.h"
#include "gui/widgets/contact-groups-configuration-widget.h"
#include "gui/widgets/contact-options-configuration-widget.h"
#include "gui/widgets/contact-personal-info-configuration-widget.h"

#include "exports.h"

class QCheckBox;
class QHostInfo;
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QScrollArea;
class QTabWidget;
class QVBoxLayout;

class Contact;


class KADUAPI ContactDataWindow : public QWidget
{
	Q_OBJECT

	Contact CurrentContact;
	ContactGeneralConfigurationWidget *ContactTab;
	ContactGroupsConfigurationWidget *GroupsTab;
	ContactPersonalInfoConfigurationWidget *PersonalInfoTab;
	ContactOptionsConfigurationWidget *OptionsTab; 

	void createGui();
	void createTabs(QLayout *layout);
	void createGeneralTab(QTabWidget *tabWidget);
	void createGroupsTab(QTabWidget *tabWidget);;
	void createPersonalInfoTab(QTabWidget *tabWidget);
	void createOptionsTab(QTabWidget *tabWidget);
	void createButtons(QLayout *layout);

	void keyPressEvent(QKeyEvent *);

private slots:
	void updateContact();
	void updateContactAndClose();

public:
	ContactDataWindow(Contact contact, QWidget *parent = 0);
	virtual ~ContactDataWindow();

};

#endif // CONTACT_DATA_WINDOW_H
