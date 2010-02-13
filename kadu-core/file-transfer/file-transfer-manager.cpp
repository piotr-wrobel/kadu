/*
 * %kadu copyright begin%
 * Copyright 2007, 2008, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2009 Piotr Galiszewski (piotrgaliszewski@gmail.com)
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

#include <QtCore/QFileInfo>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include "accounts/account.h"
#include "chat/chat-manager.h"
#include "configuration/configuration-file.h"
#include "core/core.h"
#include "file-transfer/file-transfer.h"
#include "file-transfer/file-transfer-actions.h"
#include "file-transfer/file-transfer-handler.h"
#include "file-transfer/file-transfer-notifications.h"
#include "gui/windows/file-transfer-window.h"
#include "gui/windows/kadu-window.h"
#include "gui/windows/message-dialog.h"
#include "notify/notification-manager.h"
#include "protocols/protocol.h"
#include "protocols/services/file-transfer-service.h"
#include "storage/storage-point.h"
#include "debug.h"
#include "modules.h"

#include "file-transfer-manager.h"

FileTransferManager * FileTransferManager::Instance = 0;

FileTransferManager * FileTransferManager::instance()
{
	if (0 == Instance)
		Instance = new FileTransferManager();

	return Instance;
}

FileTransferManager::FileTransferManager() :
		Window(0)
{
	Actions = new FileTransferActions(this);
	NewFileTransferNotification::registerEvents();

	triggerAllAccountsRegistered();
}

FileTransferManager::~FileTransferManager()
{
	if (Window)
	{
		delete Window;
		Window = 0;
	}

	triggerAllAccountsUnregistered();

	NewFileTransferNotification::unregisterEvents();
}

void FileTransferManager::accountRegistered(Account account)
{
	Protocol *protocol = account.protocolHandler();
	if (!protocol)
		return;

	FileTransferService *service = protocol->fileTransferService();
	if (!service)
		return;

	connect(service, SIGNAL(incomingFileTransfer(FileTransfer)),
			this, SLOT(incomingFileTransfer(FileTransfer)));
}

void FileTransferManager::accountUnregistered(Account account)
{
	Protocol *protocol = account.protocolHandler();
	if (!protocol)
		return;

	FileTransferService *service = protocol->fileTransferService();
	if (!service)
		return;

	disconnect(service, SIGNAL(incomingFileTransfer(FileTransfer)),
			this, SLOT(incomingFileTransfer(FileTransfer)));
}

void FileTransferManager::cleanUp()
{
	QList<FileTransfer> toRemove;

	foreach (FileTransfer fileTransfer, items())
		if (StatusFinished == fileTransfer.transferStatus())
			toRemove.append(fileTransfer);

	foreach (FileTransfer fileTransfer, toRemove)
		removeItem(fileTransfer);
}

void FileTransferManager::acceptFileTransfer(FileTransfer transfer)
{
	QString fileName = transfer.localFileName();

	bool haveFileName = !fileName.isEmpty();
	bool resume = haveFileName;

	QFileInfo fi;

	while (true)
	{
		if (fileName.isEmpty())
			fileName = QFileDialog::getSaveFileName(Core::instance()->kaduWindow(), tr("Select file location"),
					config_file.readEntry("Network", "LastDownloadDirectory") + transfer.remoteFileName(),
							QString::null, 0, QFileDialog::DontConfirmOverwrite);

		if (fileName.isEmpty())
		{
			kdebugmf(KDEBUG_INFO, "rejected\n");
			if (transfer.handler())
				transfer.handler()->reject();
			return;
		}

		config_file.writeEntry("Network", "LastDownloadDirectory", QFileInfo(fileName).absolutePath() + '/');
		fi.setFile(fileName);

		if (!haveFileName && fi.exists())
		{
			QString question;
			question = tr("File %1 already exists.").arg(fileName);

			switch (QMessageBox::question(0, tr("Save file"), question, tr("Overwrite"), tr("Resume"),
			                                 tr("Select another file"), 0, 2))
			{
				case 0:
					resume = false;
					break;

				case 1:
					resume = true;
					break;

				case 2:
					continue;
			}
		}

		QFile file(fileName);
		QIODevice::OpenMode flags = QIODevice::WriteOnly;
		if (resume)
			flags |= QIODevice::Append;
		else
			flags |= QIODevice::Truncate;

		if (!file.open(flags))
		{
			MessageDialog::msg(tr("Could not open file. Select another one."), true, "32x32/dialog-warning.png");
			fileName = QString::null;
		}

		transfer.createHandler();
		if (transfer.handler())
		{
			if (!transfer.handler()->accept(file))
			{
				MessageDialog::msg(tr("Could not open file. Select another one."), true, "32x32/dialog-warning.png");
				fileName = QString::null;
				continue;
			}
		}

		break;
	}

	FileTransferManager::instance()->addItem(transfer);
	transfer.setTransferStatus(StatusTransfer);
}

void FileTransferManager::rejectFileTransfer(FileTransfer transfer)
{
	if (transfer.handler())
		transfer.handler()->reject();
}

void FileTransferManager::fileTransferWindowDestroyed()
{
	Window = 0;
}

void FileTransferManager::showFileTransferWindow()
{
	if (!Window)
	{
		Window = new FileTransferWindow();
		connect(Window, SIGNAL(destroyed(QObject *)), this, SLOT(fileTransferWindowDestroyed()));
	}
	Window->show();
}

void FileTransferManager::hideFileTransferWindow()
{
	if (Window)
	{
		disconnect(Window, SIGNAL(destroyed(QObject *)), this, SLOT(fileTransferWindowDestroyed()));
		delete Window;
		Window = 0;
	}
}

bool FileTransferManager::isFileTransferWindowVisible()
{
	return Window && Window->isVisible();
}

FileTransfer FileTransferManager::byPeerAndRemoteFileName(Contact peer, const QString &remoteFileName)
{
	foreach (FileTransfer transfer, items())
		if (transfer.transferType() == TypeReceive && transfer.peer() == peer && transfer.remoteFileName() == remoteFileName)
			return transfer;

	return FileTransfer::null;
}

void FileTransferManager::incomingFileTransfer(FileTransfer fileTransfer)
{
	if (fileTransfer.localFileName().isEmpty())
	{
		FileTransfer alreadyTransferred = byPeerAndRemoteFileName(fileTransfer.peer(), fileTransfer.remoteFileName());
		if (alreadyTransferred)
			fileTransfer.setLocalFileName(alreadyTransferred.localFileName());
	}

	Chat chat = ChatManager::instance()->findChat(ContactSet(fileTransfer.peer()));
	NewFileTransferNotification *notification = new NewFileTransferNotification("FileTransfer/IncomingFile", fileTransfer,
			chat, fileTransfer.localFileName().isEmpty() ? StartNew : StartRestore);
	notification->setTitle(tr("Incoming transfer"));

	fileTransfer.setTransferStatus(StatusWaitingForAccept);

	QString textFileSize = QString("%1 kB");
	double size = (double) fileTransfer.fileSize() / 1024;

	if (size > 1024 )
	{
		size = size / 1024;
		textFileSize = "%1 MB";
	}

	// TODO: 0.8 fix that
	// we need to use \n insteadof <br />
	// <br /> are escaped inside notifications
	// \n are changed into <br />
	if (fileTransfer.localFileName().isEmpty())
		notification->setText(tr("User <b>%1</b> wants to send you a file <b>%2</b>\nof size <b>%3</b> using account <b>%4</b>.\nAccept transfer?")
				.arg(chat.name())
				.arg(fileTransfer.remoteFileName())
				.arg(textFileSize.arg(size, 0, 'f', 2))
				.arg(chat.chatAccount().name()));
	else
		notification->setText(tr("User <b>%1</b> wants to send you a file <b/>%2</b>\nof size <b>%3</b> using account <b>%4</b>.\n"
				"This is probably a next part of <b>%5</b>\n What should I do?")
				.arg(chat.name())
				.arg(fileTransfer.remoteFileName())
				.arg(textFileSize.arg(size, 0, 'f', 2))
				.arg(chat.chatAccount().name())
				.arg(fileTransfer.localFileName()));

	NotificationManager::instance()->notify(notification);
}

void FileTransferManager::itemAboutToBeAdded(FileTransfer fileTransfer)
{
	emit fileTransferAboutToBeAdded(fileTransfer);
}

void FileTransferManager::itemAdded(FileTransfer fileTransfer)
{
	emit fileTransferAdded(fileTransfer);
}

void FileTransferManager::itemAboutToBeRemoved(FileTransfer fileTransfer)
{
	emit fileTransferAboutToBeRemoved(fileTransfer);
}

void FileTransferManager::itemRemoved(FileTransfer fileTransfer)
{
	emit fileTransferRemoved(fileTransfer);
}
