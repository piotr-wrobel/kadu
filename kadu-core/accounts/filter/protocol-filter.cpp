/*
 * %kadu copyright begin%
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

#include "accounts/account.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol.h"

#include "protocol-filter.h"
#include "moc_protocol-filter.cpp"

ProtocolFilter::ProtocolFilter(QObject *parent) : AbstractAccountFilter(parent)
{
}

ProtocolFilter::~ProtocolFilter()
{
}

void ProtocolFilter::setProtocolName(const QString &protocolName)
{
    if (ProtocolName == protocolName)
        return;

    ProtocolName = protocolName;
    emit filterChanged();
}

bool ProtocolFilter::acceptAccount(Account account)
{
    return (ProtocolName.isEmpty() || ProtocolName == account.protocolName());
}
