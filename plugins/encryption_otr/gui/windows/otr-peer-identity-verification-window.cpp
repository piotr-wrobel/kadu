/*
 * %kadu copyright begin%
 * Copyright 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "otr-peer-identity-verification-window.h"
#include "moc_otr-peer-identity-verification-window.cpp"

OtrPeerIdentityVerificationWindow::OtrPeerIdentityVerificationWindow(const Contact &contact, QWidget *parent)
        : QWizard(parent), MyContact(contact)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(tr("Verify Identity of %1").arg(MyContact.display(true)));

    setOption(QWizard::NoBackButtonOnLastPage, true);
}

OtrPeerIdentityVerificationWindow::~OtrPeerIdentityVerificationWindow()
{
    emit destroyed(MyContact);
}

void OtrPeerIdentityVerificationWindow::showRespondQuestionAndAnswer(const QString &question)
{
    setStartId(RespondQuestionAndAnswerPage);
    setField("respondQuestion", question);
    show();
}

void OtrPeerIdentityVerificationWindow::showRespondSharedSecret()
{
    setStartId(RespondSharedSecretPage);
    show();
}

void OtrPeerIdentityVerificationWindow::reject()
{
    emit aboutToBeRejected();

    QWizard::reject();
}
