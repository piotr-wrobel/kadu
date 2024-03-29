/*
 * %kadu copyright begin%
 * Copyright 2011 Tomasz Rostanski (rozteck@interia.pl)
 * Copyright 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "network-access-manager-wrapper.h"
#include "moc_network-access-manager-wrapper.cpp"

#include "scripts/network-reply-wrapper.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "network/proxy/network-proxy-manager.h"

#include <QtCore/QUrl>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtScript/QScriptEngine>

NetworkAccessManagerWrapper::NetworkAccessManagerWrapper(QScriptEngine *engine, QObject *parent)
        : QNetworkAccessManager(parent), Engine(engine), Utf8(false)
{
}

NetworkAccessManagerWrapper::~NetworkAccessManagerWrapper()
{
}

void NetworkAccessManagerWrapper::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void NetworkAccessManagerWrapper::setNetworkProxyManager(NetworkProxyManager *networkProxyManager)
{
    m_networkProxyManager = networkProxyManager;
}

void NetworkAccessManagerWrapper::init()
{
    configurationUpdated();
}

void NetworkAccessManagerWrapper::configurationUpdated()
{
    NetworkProxy networkProxy;

    if (m_configuration->deprecatedApi()->readBoolEntry("SMS", "DefaultProxy", true))
        networkProxy = m_networkProxyManager->defaultProxy();
    else
        networkProxy = m_networkProxyManager->byUuid(m_configuration->deprecatedApi()->readEntry("SMS", "Proxy"));

    QNetworkProxy proxy;

    if (networkProxy)
    {
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(networkProxy.address());
        proxy.setPort(networkProxy.port());
        proxy.setUser(networkProxy.user());
        proxy.setPassword(networkProxy.password());
    }
    else
        proxy.setType(QNetworkProxy::NoProxy);

    setProxy(proxy);
}

QScriptValue NetworkAccessManagerWrapper::get(const QString &url)
{
    return Engine->newQObject(new NetworkReplyWrapper(QNetworkAccessManager::get(QNetworkRequest(url))));
}

void NetworkAccessManagerWrapper::setHeader(const QString &headerName, const QString &headerValue)
{
    // Note that QtScript by default doesn't support conversion to QByteArray,
    // so we cannot simply convert arguments to QByteArray.
    Headers.insert(headerName.toAscii(), headerValue.toAscii());
}

void NetworkAccessManagerWrapper::clearHeaders()
{
    Headers.clear();
}

QScriptValue NetworkAccessManagerWrapper::post(const QString &url, const QString &data)
{
    QNetworkRequest request;
    request.setUrl(url);
    for (QMap<QByteArray, QByteArray>::const_iterator i = Headers.constBegin(); i != Headers.constEnd(); ++i)
        request.setRawHeader(i.key(), i.value());

    QByteArray requestData;
    if (Utf8)
        requestData = data.toUtf8();
    else
        requestData = data.toAscii();

    return Engine->newQObject(new NetworkReplyWrapper(QNetworkAccessManager::post(request, requestData)));
}
