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

#include "jabber-change-password-service.h"
#include "moc_jabber-change-password-service.cpp"

#include "qxmpp/jabber-register-extension.h"
#include "services/jabber-change-password.h"
#include "services/jabber-error-service.h"

JabberChangePasswordService::JabberChangePasswordService(JabberRegisterExtension *registerExtension, QObject *parent)
        : QObject{parent}, m_registerExtension{registerExtension}
{
}

JabberChangePasswordService::~JabberChangePasswordService()
{
}

void JabberChangePasswordService::setErrorService(JabberErrorService *errorService)
{
    m_errorService = errorService;
}

JabberChangePassword *JabberChangePasswordService::changePassword(const QString &jid, const QString &newPassword)
{
    auto result = new JabberChangePassword{jid, newPassword, m_registerExtension, this};
    result->setErrorService(m_errorService);
    return result;
}
