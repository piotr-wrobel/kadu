/*
 * %kadu copyright begin%
 * Copyright 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "gui/widgets/otr-peer-identity-verification-fingerprint-exchange-page.h"
#include "gui/widgets/otr-peer-identity-verification-progress-page.h"
#include "gui/widgets/otr-peer-identity-verification-question-and-answer-page.h"
#include "gui/widgets/otr-peer-identity-verification-shared-secret-page.h"
#include "gui/windows/otr-peer-identity-verification-window.h"
#include "otr-app-ops-wrapper.h"
#include "otr-fingerprint-service.h"
#include "otr-peer-identity-verification-service.h"

#include "otr-peer-identity-verification-window-repository.h"

OtrPeerIdentityVerificationWindowRepository::OtrPeerIdentityVerificationWindowRepository(QObject *parent) :
		QObject(parent)
{
}

OtrPeerIdentityVerificationWindowRepository::~OtrPeerIdentityVerificationWindowRepository()
{
}

void OtrPeerIdentityVerificationWindowRepository::setAppOpsWrapper(OtrAppOpsWrapper *appOpsWrapper)
{
	AppOpsWrapper = appOpsWrapper;
}

void OtrPeerIdentityVerificationWindowRepository::setFingerprintService(OtrFingerprintService *fingerprintService)
{
	FingerprintService = fingerprintService;
}

void OtrPeerIdentityVerificationWindowRepository::setPeerIdentityVerificationService(OtrPeerIdentityVerificationService *peerIdentityVerificationService)
{
	PeerIdentityVerificationService = peerIdentityVerificationService;
}

OtrPeerIdentityVerificationWindow * OtrPeerIdentityVerificationWindowRepository::windowForContact(const Contact &contact)
{
	if (Windows.contains(contact))
		return Windows.value(contact);

	OtrPeerIdentityVerificationWindow *result = new OtrPeerIdentityVerificationWindow(contact);

	OtrPeerIdentityVerificationFingerprintExchangePage *fingerprintExchangePage = new OtrPeerIdentityVerificationFingerprintExchangePage(contact, result);
	fingerprintExchangePage->setFingerprintService(FingerprintService.data());
	result->setPage(OtrPeerIdentityVerificationWindow::FingerprintExchangePage, fingerprintExchangePage);

	OtrPeerIdentityVerificationQuestionAndAnswerPage *questionAndAnswerPage = new OtrPeerIdentityVerificationQuestionAndAnswerPage(contact, result);
	questionAndAnswerPage->setAppOpsWrapper(AppOpsWrapper.data());
	result->setPage(OtrPeerIdentityVerificationWindow::QuestionAndAnswerPage, questionAndAnswerPage);

	OtrPeerIdentityVerificationSharedSecretPage *sharedSecretPage = new OtrPeerIdentityVerificationSharedSecretPage(contact, result);
	sharedSecretPage->setAppOpsWrapper(AppOpsWrapper.data());
	result->setPage(OtrPeerIdentityVerificationWindow::SharedSecretPage, sharedSecretPage);

	OtrPeerIdentityVerificationProgressPage *progressPage = new OtrPeerIdentityVerificationProgressPage(contact, result);
	progressPage->setPeerIdentityVerificationService(PeerIdentityVerificationService.data());
	connect(PeerIdentityVerificationService.data(), SIGNAL(contactStateUpdated(Contact,OtrPeerIdentityVerificationState)),
			progressPage, SLOT(updateContactState(Contact,OtrPeerIdentityVerificationState)));
	result->setPage(OtrPeerIdentityVerificationWindow::ProgressPage, progressPage);

	connect(result, SIGNAL(destroyed(Contact)), this, SLOT(windowDestroyed(Contact)));
	Windows.insert(contact, result);

	return result;
}

void OtrPeerIdentityVerificationWindowRepository::windowDestroyed(const Contact &contact)
{
	Windows.remove(contact);
}

void OtrPeerIdentityVerificationWindowRepository::showVerificationWindow(const Contact &contact)
{
	OtrPeerIdentityVerificationWindow *window = windowForContact(contact);
	if (window)
		window->show();
}
