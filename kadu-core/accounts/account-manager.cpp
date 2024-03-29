/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "accounts/account-storage.h"
#include "accounts/accounts-aware-object.h"
#include "buddies/buddy-manager.h"
#include "chat/chat-manager.h"
#include "configuration/configuration-api.h"
#include "configuration/configuration-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "contacts/contact-manager.h"
#include "core/injected-factory.h"
#include "core/myself.h"
#include "identities/identity.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol.h"
#include "protocols/protocols-manager.h"
#include "roster/roster-service.h"
#include "widgets/dialog/password-dialog-widget.h"
#include "windows/kadu-dialog.h"

#include "account-manager.h"
#include "moc_account-manager.cpp"

AccountManager::AccountManager(QObject *parent) : Manager<Account>{parent}
{
}

AccountManager::~AccountManager()
{
}

void AccountManager::setAccountStorage(AccountStorage *accountStorage)
{
    m_accountStorage = accountStorage;
}

void AccountManager::setBuddyManager(BuddyManager *buddyManager)
{
    m_buddyManager = buddyManager;
}

void AccountManager::setChatManager(ChatManager *chatManager)
{
    m_chatManager = chatManager;
}

void AccountManager::setConfigurationManager(ConfigurationManager *configurationManager)
{
    m_configurationManager = configurationManager;
}

void AccountManager::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

void AccountManager::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void AccountManager::setMyself(Myself *myself)
{
    m_myself = myself;
}

void AccountManager::init()
{
    // needed for QueuedConnection
    qRegisterMetaType<Account>("Account");
    m_configurationManager->registerStorableObject(this);
}

void AccountManager::done()
{
    m_configurationManager->unregisterStorableObject(this);
}

Account AccountManager::loadStubFromStorage(const std::shared_ptr<StoragePoint> &storagePoint)
{
    return m_accountStorage->loadStubFromStorage(storagePoint);
}

void AccountManager::itemAboutToBeAdded(Account item)
{
    QMutexLocker locker(&mutex());

    if (item.data())
        item.data()->ensureLoaded();
    connect(item, SIGNAL(updated()), this, SLOT(accountDataUpdated()));
    emit accountAboutToBeAdded(item);
}

void AccountManager::itemAdded(Account item)
{
    QMutexLocker locker(&mutex());

    if (item.data())
        item.data()->ensureLoaded();
    AccountsAwareObject::notifyAccountAdded(item);
    emit accountAdded(item);
    connect(item, SIGNAL(protocolHandlerChanged(Account)), this, SLOT(protocolHandlerChanged(Account)));
    protocolHandlerChanged(item);
}

void AccountManager::itemAboutToBeRemoved(Account item)
{
    QMutexLocker locker(&mutex());

    emit accountAboutToBeRemoved(item);
}

void AccountManager::itemRemoved(Account item)
{
    QMutexLocker locker(&mutex());

    AccountsAwareObject::notifyAccountRemoved(item);
    emit accountRemoved(item);
    emit accountLoadedStateChanged(item);
    disconnect(item, 0, this, 0);
}

void AccountManager::protocolHandlerChanged(Account item)
{
    if (protocol(item))
        connect(
            protocol(item), SIGNAL(invalidPassword(Account)), this, SLOT(providePassword(Account)),
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    emit accountLoadedStateChanged(item);
}

Account AccountManager::defaultAccount()
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    // TODO: hack
    for (auto const &account : items())
        if (account.protocolName() == "gadu")
            return account;

    return byIndex(0);
}

Account AccountManager::bestAccount()
{
    return bestAccount(items());
}

const QVector<Account> AccountManager::byIdentity(Identity identity)
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    QVector<Account> list;
    for (auto const &account : items())
        if (account.accountIdentity() == identity)
            list.append(account);

    return list;
}

Account AccountManager::byId(const QString &protocolName, const QString &id)
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    for (auto const &account : items())
        if (account.protocolName() == protocolName && account.id() == id)
            return account;

    return Account::null;
}

const QVector<Account> AccountManager::byProtocolName(const QString &name)
{
    QMutexLocker locker(&mutex());

    ensureLoaded();

    QVector<Account> list;
    for (auto const &account : items())
        if (account.protocolName() == name)
            list.append(account);

    return list;
}

void AccountManager::accountDataUpdated()
{
    QMutexLocker locker(&mutex());

    Account account(sender());
    if (account)
        emit accountUpdated(account);
}

void AccountManager::removeAccountAndBuddies(Account account)
{
    auto statusContainer = account.statusContainer();
    if (statusContainer)
        statusContainer->setStatus(Status(), SourceUser);   // user removed account

    if (auto p = protocol(account))
        delete p->rosterService();

    removeItem(account);

    for (auto const &contact : m_contactManager->contacts(account))
        m_buddyManager->clearOwnerAndRemoveEmptyBuddy(contact);

    for (auto const &chat : m_chatManager->chats(account))
        chat.setDisplay(QString());
}

void AccountManager::passwordProvided(const QVariant &data, const QString &password, bool permanent)
{
    Account account = data.value<Account>();
    if (!account)
        return;

    account.setPassword(password);
    account.setRememberPassword(permanent);
    account.setHasPassword(!password.isEmpty());

    // inform protocol that we have password
    // maybe this should be in other place, but for now it is enough
    if (auto p = protocol(account))
        p->passwordProvided();
}

void AccountManager::providePassword(Account account)
{
    QMutexLocker locker(&mutex());

    QString message =
        tr("Please provide password for %1 (%2) account").arg(account.accountIdentity().name()).arg(account.id());

    auto passwordWidget = m_injectedFactory->makeInjected<PasswordDialogWidget>(message, account, nullptr);
    connect(
        passwordWidget, SIGNAL(passwordEntered(const QVariant &, const QString &, bool)), this,
        SLOT(passwordProvided(const QVariant &, const QString &, bool)));
    KaduDialog *window = new KaduDialog(passwordWidget, 0);
    window->exec();
}

void AccountManager::loaded()
{
    Manager<Account>::loaded();

    for (auto const &account : items())
        account.accountContact().setOwnerBuddy(m_myself->buddy());
}
