/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2011, 2013, 2014 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#pragma once

#include "contacts/contact.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class OtrFingerprintService;
class OtrPeerIdentityVerificationService;
class OtrPeerIdentityVerificationWindow;
class OtrTrustLevelService;

class OtrPeerIdentityVerificationWindowFactory : public QObject
{
	Q_OBJECT

public:
	Q_INVOKABLE OtrPeerIdentityVerificationWindowFactory();
	virtual ~OtrPeerIdentityVerificationWindowFactory();

	OtrPeerIdentityVerificationWindow * windowForContact(const Contact &contact);

private slots:
	INJEQT_SETTER void setFingerprintService(OtrFingerprintService *fingerprintService);
	INJEQT_SETTER void setPeerIdentityVerificationService(OtrPeerIdentityVerificationService *peerIdentityVerificationService);
	INJEQT_SETTER void setTrustLevelService(OtrTrustLevelService *trustLevelService);

private:
	QPointer<OtrFingerprintService> FingerprintService;
	QPointer<OtrPeerIdentityVerificationService> PeerIdentityVerificationService;
	QPointer<OtrTrustLevelService> TrustLevelService;

	QMap<Contact, OtrPeerIdentityVerificationWindow *> Windows;

};
