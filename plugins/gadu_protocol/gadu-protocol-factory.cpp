/*
 * %kadu copyright begin%
 * Copyright 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtWidgets/QPushButton>

#include "core/core.h"
#include "icons/kadu-icon.h"
#include "status/status-type.h"

#include "gadu-id-validator.h"
#include "gadu-protocol.h"
#include "gui/widgets/gadu-add-account-widget.h"
#include "gui/widgets/gadu-contact-personal-info-widget.h"
#include "gui/widgets/gadu-edit-account-widget.h"
#include "helpers/gadu-list-helper.h"
#include "plugin/plugin-injected-factory.h"

#include "gadu-protocol-factory.h"
#include "moc_gadu-protocol-factory.cpp"

GaduProtocolFactory::GaduProtocolFactory(QObject *parent) : ProtocolFactory{}
{
    Q_UNUSED(parent);

    // already sorted
    SupportedStatusTypes.append(StatusType::FreeForChat);
    SupportedStatusTypes.append(StatusType::Online);
    SupportedStatusTypes.append(StatusType::Away);
    SupportedStatusTypes.append(StatusType::DoNotDisturb);
    SupportedStatusTypes.append(StatusType::Invisible);
    SupportedStatusTypes.append(StatusType::Offline);
}

GaduProtocolFactory::~GaduProtocolFactory()
{
}

void GaduProtocolFactory::setGaduListHelper(GaduListHelper *gaduListHelper)
{
    m_gaduListHelper = gaduListHelper;
}

void GaduProtocolFactory::setGaduServersManager(GaduServersManager *gaduServersManager)
{
    m_gaduServersManager = gaduServersManager;
}

void GaduProtocolFactory::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

Protocol *GaduProtocolFactory::createProtocolHandler(Account account)
{
    return m_pluginInjectedFactory->makeInjected<GaduProtocol>(m_gaduListHelper, m_gaduServersManager, account, this);
}

AccountAddWidget *GaduProtocolFactory::newAddAccountWidget(bool showButtons, QWidget *parent)
{
    auto result = m_pluginInjectedFactory->makeInjected<GaduAddAccountWidget>(showButtons, parent);
    connect(this, SIGNAL(destroyed()), result, SLOT(deleteLater()));
    return result;
}

AccountCreateWidget *GaduProtocolFactory::newCreateAccountWidget(bool, QWidget *)
{
    return nullptr;
}

AccountEditWidget *GaduProtocolFactory::newEditAccountWidget(Account account, QWidget *parent)
{
    auto result = m_pluginInjectedFactory->makeInjected<GaduEditAccountWidget>(m_gaduServersManager, account, parent);
    connect(this, SIGNAL(destroyed()), result, SLOT(deleteLater()));
    return result;
}

QList<StatusType> GaduProtocolFactory::supportedStatusTypes()
{
    return SupportedStatusTypes;
}

Status GaduProtocolFactory::adaptStatus(Status status) const
{
    Status adapted = status;

    if (adapted.type() == StatusType::NotAvailable)
        adapted.setType(StatusType::Away);

    return adapted;
}

QString GaduProtocolFactory::idLabel()
{
    return tr("Gadu-Gadu number:");
}

QValidator::State GaduProtocolFactory::validateId(QString id)
{
    int pos = 0;
    return createNotOwnedGaduIdValidator()->validate(id, pos);
}

bool GaduProtocolFactory::canRegister()
{
    return false;
}

QWidget *GaduProtocolFactory::newContactPersonalInfoWidget(Contact contact, QWidget *parent)
{
    return new GaduContactPersonalInfoWidget(contact, parent);
}

KaduIcon GaduProtocolFactory::icon()
{
    return KaduIcon("protocols/gadu-gadu/gadu-gadu");
}
