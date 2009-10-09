/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QTimer>
#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QIntValidator>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QScrollBar>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtNetwork/QHostInfo>

#include "accounts/account.h"
#include "accounts/account-manager.h"
#include "configuration/configuration-file.h"
#include "configuration/xml-configuration-file.h"
#include "contacts/contact-account-data.h"
#include "contacts/contact-manager.h"
#include "contacts/group.h"
#include "contacts/group-manager.h"
#include "gui/widgets/contact-general-configuration-widget.h"
#include "gui/widgets/contact-groups-configuration-widget.h"
#include "gui/widgets/contact-options-configuration-widget.h"
#include "gui/widgets/contact-personal-info-configuration-widget.h"
#include "gui/windows/contact-data-manager.h"
#include "gui/windows/message-box.h"
#include "protocols/protocol.h"
#include "protocols/protocol-factory.h"

#include "debug.h"
#include "icons-manager.h"
#include "misc/misc.h"

#include "contact-data-window.h"

ContactDataWindow::ContactDataWindow(Contact contact, QWidget *parent) :
		QWidget(parent, Qt::Dialog), CurrentContact(contact)
{
	kdebugf();

	setAttribute(Qt::WA_DeleteOnClose);

	createGui();

	setWindowTitle(tr("Merged Contact Properties - %1").arg(CurrentContact.display()));

	loadWindowGeometry(this, "General", "ManageUsersDialogGeometry", 0, 50, 425, 500);
}

ContactDataWindow::~ContactDataWindow()
{
	kdebugf();
 	saveWindowGeometry(this, "General", "ManageUsersDialogGeometry");
	kdebugf2();
}

void ContactDataWindow::createGui()
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	createTabs(layout);
	createButtons(layout);
}

void ContactDataWindow::createTabs(QLayout *layout)
{
	QTabWidget *tabWidget = new QTabWidget(this);

	createGeneralTab(tabWidget);
	createGroupsTab(tabWidget);
	createPersonalInfoTab(tabWidget);
	createOptionsTab(tabWidget);
	layout->addWidget(tabWidget);
}

void ContactDataWindow::createGeneralTab(QTabWidget *tabWidget)
{
	ContactGeneralConfigurationWidget *contactTab = new ContactGeneralConfigurationWidget(CurrentContact, this);
	contactTab->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	tabWidget->addTab(contactTab, tr("General"));
}

void ContactDataWindow::createGroupsTab(QTabWidget *tabWidget)
{
	ContactGroupsConfigurationWidget *groupsTab = new ContactGroupsConfigurationWidget(CurrentContact, this);
	groupsTab->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	tabWidget->addTab(groupsTab, tr("Groups"));
}

void ContactDataWindow::createPersonalInfoTab(QTabWidget *tabWidget)
{
	ContactPersonalInfoConfigurationWidget *personalInfoTab = new ContactPersonalInfoConfigurationWidget(this);
	personalInfoTab->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	tabWidget->addTab(personalInfoTab, tr("Personal Information"));
}

void ContactDataWindow::createOptionsTab(QTabWidget *tabWidget)
{
	ContactOptionsConfigurationWidget *optionsTab = new ContactOptionsConfigurationWidget(CurrentContact, this);
	optionsTab->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	tabWidget->addTab(optionsTab, tr("Options"));
}

/*
void ContactDataWindow::createOptionsTab(QTabWidget *tabWidget)
{
// 	foreach (Account *account, CurrentContact.accounts())
// 		createAccountTab(account, tabWidget);
}

void ContactDataWindow::createAccountTab(Account *account, QTabWidget *tabWidget)
{
	if (!account || !account->protocol())
		return;

	ProtocolFactory *protocolFactory = account->protocol()->protocolFactory();
	ContactAccountData *contactAccountData = CurrentContact.accountData(account);

	if (!contactAccountData || !protocolFactory)
		return;

	ContactAccountDataWidget *contactAccountDataWidget = protocolFactory->newContactAccountDataWidget(contactAccountData, this);
	if (!contactAccountDataWidget)
		return;

	contactAccountDataWidget->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	contactAccountDataWidget->loadConfiguration();

	ConfigurationWidgets.append(contactAccountDataWidget);
	tabWidget->addTab(contactAccountDataWidget, account->name());
}
*/

void ContactDataWindow::createButtons(QLayout *layout)
{
	QDialogButtonBox *buttons = new QDialogButtonBox(Qt::Horizontal, this);

	QPushButton *cancelButton = new QPushButton(IconsManager::instance()->loadIcon("CloseWindowButton"), tr("Cancel"), this);
	buttons->addButton(cancelButton, QDialogButtonBox::RejectRole);
	QPushButton *saveButton = new QPushButton(IconsManager::instance()->loadIcon("OkWindowButton"), tr("Save"), this);
	buttons->addButton(saveButton, QDialogButtonBox::AcceptRole);

	connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(updateContactAndClose()));
	connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));

	layout->addWidget(buttons);
}

void ContactDataWindow::updateContact()
{
	ContactManager::instance()->blockUpdatedSignal(CurrentContact);

	foreach (ConfigurationWidget *configurationWidget, ConfigurationWidgets)
		configurationWidget->saveConfiguration();

	ContactManager::instance()->unblockUpdatedSignal(CurrentContact);
}

void ContactDataWindow::updateContactAndClose()
{
	updateContact();
	close();
}

void ContactDataWindow::keyPressEvent(QKeyEvent *ke_event)
{
	if (ke_event->key() == Qt::Key_Escape)
		close();
}
