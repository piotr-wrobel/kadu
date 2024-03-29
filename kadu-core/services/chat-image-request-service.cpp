/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QTimer>

#include "accounts/account-manager.h"
#include "contacts/contact-manager.h"
#include "icons/icons-manager.h"
#include "protocols/services/chat-image-service.h"
#include "services/image-storage-service.h"
#include "windows/message-dialog.h"

#include "chat-image-request-service.h"
#include "moc_chat-image-request-service.cpp"

ChatImageRequestService::ChatImageRequestService(QObject *parent) : QObject(parent), ReceivedImageKeysCount(0)
{
    QTimer *everyMinuteTimer = new QTimer(this);
    everyMinuteTimer->setInterval(60 * 1000);
    everyMinuteTimer->setSingleShot(false);
    connect(everyMinuteTimer, SIGNAL(timeout()), this, SLOT(resetReceivedImageKeysCount()));
    everyMinuteTimer->start();
}

ChatImageRequestService::~ChatImageRequestService()
{
}

void ChatImageRequestService::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void ChatImageRequestService::setImageStorageService(ImageStorageService *imageStorageService)
{
    CurrentImageStorageService = imageStorageService;
}

void ChatImageRequestService::setAccountManager(AccountManager *accountManager)
{
    if (CurrentAccountManager)
        disconnect(CurrentAccountManager.data(), 0, this, 0);

    CurrentAccountManager = accountManager;
    if (!CurrentAccountManager)
        return;

    connect(
        CurrentAccountManager.data(), SIGNAL(accountLoadedStateChanged(Account)), this, SLOT(connectAccount(Account)));
}

void ChatImageRequestService::setContactManager(ContactManager *contactManager)
{
    CurrentContactManager = contactManager;
}

void ChatImageRequestService::setConfiguration(ChatImageRequestServiceConfiguration configuration)
{
    Configuration = configuration;
}

void ChatImageRequestService::connectAccount(Account account)
{
    if (!account || !account.protocolHandler() || !account.protocolHandler()->chatImageService())
        return;

    connect(
        account.protocolHandler()->chatImageService(), SIGNAL(chatImageKeyReceived(QString, ChatImage)), this,
        SLOT(chatImageKeyReceived(QString, ChatImage)), Qt::UniqueConnection);
    connect(
        account.protocolHandler()->chatImageService(), SIGNAL(chatImageAvailable(ChatImage, QByteArray)), this,
        SLOT(chatImageAvailable(ChatImage, QByteArray)), Qt::UniqueConnection);
}

bool ChatImageRequestService::acceptImage(const Account &account, const QString &id, const ChatImage &chatImage) const
{
    if (!Configuration.limitImageSize())
        return true;

    if (Configuration.maximumImageSizeInKiloBytes() * 1024 >= chatImage.size())
        return true;

    if (!CurrentContactManager)
        return false;

    if (!Configuration.allowBiggerImagesAfterAsking())
        return false;

    const Contact &contact = CurrentContactManager.data()->byId(account, id, ActionReturnNull);

    QString question = tr("Buddy %1 is attempting to send you an image of %2 KiB in size.\n"
                          "This exceeds your configured limits.\n"
                          "Do you want to accept this image anyway?")
                           .arg(contact.display(true))
                           .arg((chatImage.size() + 1023) / 1024);

    MessageDialog *dialog = MessageDialog::create(
        m_iconsManager->iconByPath(KaduIcon("dialog-question")), tr("Kadu") + " - " + tr("Incoming Image"), question);
    dialog->addButton(QMessageBox::Yes, tr("Accept image"));
    dialog->addButton(QMessageBox::No, tr("Deny"));

    return dialog->ask();
}

void ChatImageRequestService::chatImageKeyReceived(const QString &id, const ChatImage &chatImage)
{
    if (ReceivedImageKeysCount >= ReceivedImageKeysPerMinuteLimit)
        return;

    ChatImageService *service = qobject_cast<ChatImageService *>(sender());
    if (!service)
        return;

    if (!acceptImage(service->account(), id, chatImage))
        return;

    service->requestChatImage(id, chatImage);
    ReceivedImageKeysCount++;
}

void ChatImageRequestService::chatImageAvailable(const ChatImage &chatImage, const QByteArray &imageData)
{
    if (!CurrentImageStorageService)
        return;

    QString filePath = CurrentImageStorageService.data()->storeImage(chatImage.key(), imageData);
    emit chatImageStored(chatImage, CurrentImageStorageService.data()->fullPath(filePath));
}

void ChatImageRequestService::resetReceivedImageKeysCount()
{
    ReceivedImageKeysCount = 0;
}
