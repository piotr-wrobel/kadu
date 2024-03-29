/*
 * %kadu copyright begin%
 * Copyright 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtXml/QDomElement>

#include "configuration/configuration-api.h"
#include "configuration/configuration-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "network/proxy/network-proxy-storage.h"
#include "storage/storage-point.h"

#include "network-proxy-manager.h"
#include "moc_network-proxy-manager.cpp"

NetworkProxyManager::NetworkProxyManager(QObject *parent) : Manager<NetworkProxy>{parent}
{
}

NetworkProxyManager::~NetworkProxyManager()
{
}

void NetworkProxyManager::setConfigurationManager(ConfigurationManager *configurationManager)
{
    m_configurationManager = configurationManager;
}

void NetworkProxyManager::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void NetworkProxyManager::setNetworkProxyStorage(NetworkProxyStorage *networkProxyStorage)
{
    m_networkProxyStorage = networkProxyStorage;
}

void NetworkProxyManager::init()
{
    m_configurationManager->registerStorableObject(this);
    configurationUpdated();
}

void NetworkProxyManager::done()
{
    m_configurationManager->unregisterStorableObject(this);
}

void NetworkProxyManager::load()
{
    QMutexLocker locker(&mutex());

    Manager<NetworkProxy>::load();
}

void NetworkProxyManager::store()
{
    QMutexLocker locker(&mutex());

    Manager<NetworkProxy>::store();
}

NetworkProxy NetworkProxyManager::loadStubFromStorage(const std::shared_ptr<StoragePoint> &storagePoint)
{
    return m_networkProxyStorage->loadStubFromStorage(storagePoint);
}

void NetworkProxyManager::configurationUpdated()
{
    DefaultProxy = byUuid(m_configuration->deprecatedApi()->readEntry("Network", "DefaultProxy"));
}

void NetworkProxyManager::setDefaultProxy(const NetworkProxy &proxy)
{
    DefaultProxy = proxy;
    m_configuration->deprecatedApi()->writeEntry("Network", "DefaultProxy", DefaultProxy.uuid().toString());
}

const NetworkProxy &NetworkProxyManager::defaultProxy()
{
    return DefaultProxy;
}

NetworkProxy NetworkProxyManager::byConfiguration(
    const QString &address, int port, const QString &user, const QString &password, NotFoundAction action)
{
    for (auto const &networkProxy : items())
    {
        if (networkProxy.address() == address && networkProxy.port() == port && networkProxy.user() == user &&
            networkProxy.password() == password)
            return networkProxy;
    }

    if (ActionReturnNull == action)
        return NetworkProxy::null;

    auto networkProxy = m_networkProxyStorage->create();
    networkProxy.setAddress(address);
    networkProxy.setPort(port);
    networkProxy.setUser(user);
    networkProxy.setPassword(password);

    if (ActionCreateAndAdd == action)
        addItem(networkProxy);

    return networkProxy;
}

void NetworkProxyManager::networkProxyDataUpdated()
{
    NetworkProxy networkProxy(sender());
    if (!networkProxy.isNull())
        emit networkProxyUpdated(networkProxy);
}

void NetworkProxyManager::itemAboutToBeAdded(NetworkProxy item)
{
    connect(item, SIGNAL(updated()), this, SLOT(networkProxyDataUpdated()));
    emit networkProxyAboutToBeAdded(item);
}

void NetworkProxyManager::itemAdded(NetworkProxy item)
{
    emit networkProxyAdded(item);
}

void NetworkProxyManager::itemAboutToBeRemoved(NetworkProxy item)
{
    emit networkProxyAboutToBeRemoved(item);
}

void NetworkProxyManager::itemRemoved(NetworkProxy item)
{
    disconnect(item, 0, this, 0);
    emit networkProxyRemoved(item);
}
