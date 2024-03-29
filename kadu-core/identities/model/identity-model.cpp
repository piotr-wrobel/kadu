/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "identity-model.h"
#include "moc_identity-model.cpp"

#include "identities/identity-manager.h"
#include "model/roles.h"
#include "protocols/protocol.h"

IdentityModel::IdentityModel(QObject *parent) : QAbstractListModel(parent)
{
}

IdentityModel::~IdentityModel()
{
    disconnect(m_identityManager, 0, this, 0);
}

void IdentityModel::setIdentityManager(IdentityManager *identityManager)
{
    m_identityManager = identityManager;
}

void IdentityModel::init()
{
    connect(
        m_identityManager, SIGNAL(identityAboutToBeAdded(Identity)), this, SLOT(identityAboutToBeAdded(Identity)),
        Qt::DirectConnection);
    connect(
        m_identityManager, SIGNAL(identityAdded(Identity)), this, SLOT(identityAdded(Identity)), Qt::DirectConnection);
    connect(
        m_identityManager, SIGNAL(identityAboutToBeRemoved(Identity)), this, SLOT(identityAboutToBeRemoved(Identity)),
        Qt::DirectConnection);
    connect(
        m_identityManager, SIGNAL(identityRemoved(Identity)), this, SLOT(identityRemoved(Identity)),
        Qt::DirectConnection);
}

int IdentityModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_identityManager->count();
}

QVariant IdentityModel::data(const QModelIndex &index, int role) const
{
    Identity ident = identity(index);
    if (0 == ident)
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        return ident.name();
    case IdentityRole:
        return QVariant::fromValue(ident);
    case ItemTypeRole:
        return IdentityRole;
    default:
        return QVariant();
    }
}

Identity IdentityModel::identity(const QModelIndex &index) const
{
    if (!index.isValid())
        return Identity::null;

    if (index.row() < 0 || index.row() >= rowCount())
        return Identity::null;

    return m_identityManager->byIndex(index.row());
}

int IdentityModel::identityIndex(Identity identity) const
{
    return m_identityManager->indexOf(identity);
}

QModelIndexList IdentityModel::indexListForValue(const QVariant &value) const
{
    QModelIndexList result;

    const int i = identityIndex(value.value<Identity>());
    if (-1 != i)
        result.append(index(i, 0));

    return result;
}

void IdentityModel::identityAboutToBeAdded(Identity identity)
{
    Q_UNUSED(identity)

    int count = rowCount();
    beginInsertRows(QModelIndex(), count, count);
}

void IdentityModel::identityAdded(Identity identity)
{
    Q_UNUSED(identity)

    endInsertRows();
}

void IdentityModel::identityAboutToBeRemoved(Identity identity)
{
    int index = identityIndex(identity);
    beginRemoveRows(QModelIndex(), index, index);
}

void IdentityModel::identityRemoved(Identity identity)
{
    Q_UNUSED(identity)

    endRemoveRows();
}
