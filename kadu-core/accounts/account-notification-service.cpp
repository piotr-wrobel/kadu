/*
 * %kadu copyright begin%
 * Copyright 2016 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "account-notification-service.h"
#include "moc_account-notification-service.cpp"

#include "accounts/account.h"
#include "html/html-conversion.h"
#include "html/html-string.h"
#include "html/normalized-html-string.h"
#include "identities/identity.h"
#include "notification/notification-callback-repository.h"
#include "notification/notification-callback.h"
#include "notification/notification-event-repository.h"
#include "notification/notification-event.h"
#include "notification/notification-service.h"
#include "notification/notification.h"
#include "parser/parser.h"

#include <QtCore/QDateTime>

static QString getErrorMessage(const ParserData *const object)
{
    auto notification = dynamic_cast<const Notification *const>(object);
    if (notification)
        return notification->data[QStringLiteral("error-message")].toString();
    else
        return QString{};
}

static QString getErrorServer(const ParserData *const object)
{
    auto notification = dynamic_cast<const Notification *const>(object);
    if (notification)
        return notification->data[QStringLiteral("error-server")].toString();
    else
        return QString{};
}

AccountNotificationService::AccountNotificationService(QObject *parent)
        : QObject{parent}, m_ignoreErrorsCallback{QStringLiteral("connection-ignore-errors"), tr("Ignore"),
                                                  [this](const Notification &notification) {
                                                      return ignoreErrors(notification);
                                                  }},
          m_connectionErrorEvent{QStringLiteral("ConnectionError"),
                                 QStringLiteral(QT_TRANSLATE_NOOP("@default", "Connection error"))}
{
}

AccountNotificationService::~AccountNotificationService()
{
}

void AccountNotificationService::setNotificationCallbackRepository(
    NotificationCallbackRepository *notificationCallbackRepository)
{
    m_notificationCallbackRepository = notificationCallbackRepository;
}

void AccountNotificationService::setNotificationEventRepository(
    NotificationEventRepository *notificationEventRepository)
{
    m_notificationEventRepository = notificationEventRepository;
}

void AccountNotificationService::setNotificationService(NotificationService *notificationService)
{
    m_notificationService = notificationService;
}

void AccountNotificationService::setParser(Parser *parser)
{
    m_parser = parser;
}

void AccountNotificationService::init()
{
    m_notificationEventRepository->addNotificationEvent(m_connectionErrorEvent);
    m_notificationCallbackRepository->addCallback(m_ignoreErrorsCallback);

    m_parser->registerObjectTag(QStringLiteral("error"), getErrorMessage);
    m_parser->registerObjectTag(QStringLiteral("errorServer"), getErrorServer);
}

void AccountNotificationService::done()
{
    m_notificationEventRepository->removeNotificationEvent(m_connectionErrorEvent);
    m_notificationCallbackRepository->removeCallback(m_ignoreErrorsCallback);

    m_parser->unregisterObjectTag(QStringLiteral("error"));
    m_parser->unregisterObjectTag(QStringLiteral("errorServer"));
}

void AccountNotificationService::notifyConnectionError(
    const Account &account, const QString &errorServer, const QString &errorMessage)
{
    if (account.property(QStringLiteral("notify:ignore-connection-errors"), false).toBool())
        return;

    auto lastConnectionError =
        account.property(QStringLiteral("notify:last-connection-error"), QDateTime{}).toDateTime();
    if (lastConnectionError.isValid() && lastConnectionError.addSecs(60) > QDateTime::currentDateTime())
        return;

    account.addProperty(
        QStringLiteral("notify:last-connection-error"), QDateTime::currentDateTime(), CustomProperties::NonStorable);

    auto data = QVariantMap{};
    data.insert(QStringLiteral("account"), qVariantFromValue(account));
    data.insert(QStringLiteral("error-server"), errorServer);
    data.insert(QStringLiteral("error-message"), errorMessage);

    auto notification = Notification{};
    notification.type = m_connectionErrorEvent.name();
    notification.title = tr("Connection error");
    notification.text = normalizeHtml(
        plainToHtml(tr("Connection error on account: %1 (%2)").arg(account.id(), account.accountIdentity().name())));
    notification.data = std::move(data);
    notification.details = normalizeHtml(plainToHtml(errorDetails(errorServer, errorMessage)));
    notification.callbacks.append(QStringLiteral("connection-ignore-errors"));

    m_notificationService->notify(notification);
}

QString AccountNotificationService::errorDetails(const QString &errorServer, const QString &errorMessage)
{
    return errorMessage.isEmpty()
               ? QString{}
               : errorServer.isEmpty() ? errorMessage : QStringLiteral("%1 (%2)").arg(errorMessage, errorServer);
}

void AccountNotificationService::ignoreErrors(const Notification &notification)
{
    auto account = qvariant_cast<Account>(notification.data[QStringLiteral("account")]);
    account.addProperty(QStringLiteral("notify:ignore-connection-errors"), false, CustomProperties::NonStorable);
    connect(account, SIGNAL(connected()), this, SLOT(accountConnected()));
}

void AccountNotificationService::accountConnected()
{
    auto account = Account{sender()};
    account.removeProperty(QStringLiteral("notify:ignore-connection-errors"));
}
