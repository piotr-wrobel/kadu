/*
 * %kadu copyright begin%
 * Copyright 2012, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "protocol-service.h"
#include "moc_protocol-service.cpp"

#include "protocols/protocol.h"

ProtocolService::ProtocolService(Protocol *protocol, QObject *parent)
        : AccountService{protocol->account(), parent}, m_protocol{protocol}
{
    Q_ASSERT(m_protocol);
}

ProtocolService::~ProtocolService()
{
}

Protocol *ProtocolService::protocol() const
{
    return m_protocol;
}
