/*
 * %kadu copyright begin%
 * Copyright 2008, 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009, 2009, 2010 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2008 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2004 Adrian Smarzewski (adrian@kadu.net)
 * Copyright 2007, 2008, 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2004, 2006 Marcin Ślusarz (joi@kadu.net)
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

#ifndef JABBER_CREATE_ACCOUNT_WIDGET_H
#define JABBER_CREATE_ACCOUNT_WIDGET_H

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

#include <QtWidgets/QWidget>

#include "accounts/account.h"
#include "gui/widgets/account-create-widget.h"
#include "gui/widgets/identities-combo-box.h"

class QGridLayout;
class QLineEdit;
class QPushButton;

class ChooseIdentityWidget;
class JabberServerRegisterAccount;
class TokenWidget;

class JabberCreateAccountWidget : public AccountCreateWidget
{
	Q_OBJECT

	QLineEdit *Username;
	QComboBox *Domain;
	QLineEdit *NewPassword;
	QLineEdit *ReNewPassword;
	QCheckBox *RememberPassword;
	IdentitiesComboBox *IdentityCombo;
	QPushButton *RegisterAccountButton;

	QPushButton *ExpandConnectionOptionsButton;
	QWidget *OptionsWidget;
	QCheckBox *CustomHostPort;
	QHBoxLayout *HostPortLayout;
	QLabel *CustomHostLabel;
	QLineEdit *CustomHost;
	QLabel *CustomPortLabel;
	QLineEdit *CustomPort;
	QLabel *EncryptionModeLabel;
	QComboBox *EncryptionMode;
	QCheckBox *LegacySSLProbe;

	int ssl_;
	bool opt_host_, legacy_ssl_probe_;
	QString host_;
	uint port_;
	bool ShowConnectionOptions;

	void createGui(bool showButtons);
	bool checkSSL();
	void resetGui();

private slots:
	void dataChanged();
	void connectionOptionsChanged();
	void hostToggled(bool on);
	void sslActivated(int i);

	void jidRegistered(const QString &jid, const QString &tlsDomain);

public:
	explicit JabberCreateAccountWidget(bool showButtons, QWidget *parent = 0);
	virtual ~JabberCreateAccountWidget();

public slots:
	virtual void apply();
	virtual void cancel();

};

#endif // JABBER_CREATE_ACCOUNT_WIDGET_H
