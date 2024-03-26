/*
 * %kadu copyright begin%
 * Copyright 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "jabber-register-account-service.h"
#include "jabber-register-account-service.moc"

#include "services/jabber-error-service.h"
#include "services/jabber-register-account.h"

#include "plugin/plugin-injected-factory.h"

#include <QXmppQt5/QXmppClient.h>

JabberRegisterAccountService::JabberRegisterAccountService(QObject *parent) : QObject{parent}
{
}

JabberRegisterAccountService::~JabberRegisterAccountService()
{
}

void JabberRegisterAccountService::setErrorService(JabberErrorService *errorService)
{
    m_errorService = errorService;
}

void JabberRegisterAccountService::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

JabberRegisterAccount *JabberRegisterAccountService::registerAccount(Jid jid, QString password, QString email)
{
    auto result = m_pluginInjectedFactory->makeInjected<JabberRegisterAccount>(
        std::move(jid), std::move(password), std::move(email), this);
    result->setErrorService(m_errorService);
    return result;
}
