/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "accounts/account.h"
#include "dcc/dcc-manager.h"
#include "dcc/dcc-socket-notifiers.h"
#include "gadu-contact-account-data.h"
#include "gadu-protocol.h"

#include "gadu-file-transfer.h"

GaduFileTransfer::GaduFileTransfer(Account *account) :
		FileTransfer(account)
{
}

GaduFileTransfer::GaduFileTransfer(Account *account, Contact peer, FileTransfer::FileTransferType transferType) :
		FileTransfer(account, peer, transferType),
		SocketNotifiers(0), WaitingForSocketNotifiers(false)
{
}

void GaduFileTransfer::updateFileInfo()
{
	if (SocketNotifiers)
	{
		setFileSize(SocketNotifiers->fileSize());
		setFileSize(SocketNotifiers->transferredFileSize());
	}
	else
	{
		setFileSize(0);
		setTransferredSize(0);
	}
}

void GaduFileTransfer::setFileTransferNotifiers(DccSocketNotifiers *socketNotifiers)
{
	if (!socketNotifiers)
	{
		socketNotAvailable();
		return;
	}

	SocketNotifiers = socketNotifiers;
	SocketNotifiers->setGaduFileTransfer(this);
	WaitingForSocketNotifiers = false;
	setRemoteFile(socketNotifiers->remoteFileName());

	changeFileTransferStatus(FileTransfer::StatusTransfer);
}

void GaduFileTransfer::socketNotAvailable()
{
	WaitingForSocketNotifiers = false;

	changeFileTransferStatus(FileTransfer::StatusFinished);
}

void GaduFileTransfer::send()
{
	if (FileTransfer::TypeSend != transferType()) // maybe assert here?
		return;

	if (SocketNotifiers || WaitingForSocketNotifiers) // already sending/receiving
		return;

	setRemoteFile(QString::null);

	if (!account() || localFileName().isEmpty())
	{
		changeFileTransferStatus(FileTransfer::StatusNotConnected);
		return; // TODO: notify
	}

	GaduProtocol *gaduProtocol = dynamic_cast<GaduProtocol *>(account()->protocol());
	if (!gaduProtocol)
	{
		changeFileTransferStatus(FileTransfer::StatusNotConnected);
		return;
	}

	GaduContactAccountData *gcad = gaduProtocol->gaduContactAccountData(contact());
	if (!gcad)
	{
		changeFileTransferStatus(FileTransfer::StatusNotConnected);
		return;
	}

	// async call, will return in setFileTransferNotifiers
	changeFileTransferStatus(FileTransfer::StatusWaitingForConnection);
	WaitingForSocketNotifiers = true;
	gaduProtocol->dccManager()->attachSendFileTransferSocket(this);
}

void GaduFileTransfer::stop()
{
	if (SocketNotifiers)
	{
		delete SocketNotifiers;
		SocketNotifiers = 0;
		changeFileTransferStatus(FileTransfer::StatusNotConnected);
	}
}

void GaduFileTransfer::pause()
{
	stop();
}

void GaduFileTransfer::restore()
{
	if (FileTransfer::TypeSend == transferType())
		send();
}
