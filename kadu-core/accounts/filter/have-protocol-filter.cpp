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

#include "have-protocol-filter.h"
#include "moc_have-protocol-filter.cpp"

#include "accounts/account.h"

HaveProtocolFilter::HaveProtocolFilter(QObject *parent) : AbstractAccountFilter{parent}
{
}

HaveProtocolFilter::~HaveProtocolFilter()
{
}

bool HaveProtocolFilter::acceptAccount(Account account)
{
    return account.protocolHandler();
}
