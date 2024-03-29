/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "contact-list-model.h"
#include "moc_contact-list-model.cpp"

#include "avatars/avatar-id.h"
#include "avatars/avatars.h"
#include "contacts/model/contact-data-extractor.h"

ContactListModel::ContactListModel(QObject *parent) : QAbstractItemModel(parent)
{
}

ContactListModel::~ContactListModel()
{
}

void ContactListModel::setAvatars(Avatars *avatars)
{
    connect(avatars, &Avatars::updated, this, &ContactListModel::avatarUpdated);
}

void ContactListModel::setContactDataExtractor(ContactDataExtractor *contactDataExtractor)
{
    m_contactDataExtractor = contactDataExtractor;
}

void ContactListModel::avatarUpdated(const AvatarId &id)
{
    auto it = std::find_if(std::begin(m_list), std::end(m_list), [&id](const Contact &c) { return avatarId(c) == id; });
    if (it != std::end(m_list))
    {
        auto row = std::distance(std::begin(m_list), it);
        auto const &contactIndex = index(row, 0);
        emit dataChanged(contactIndex, contactIndex);
    }
}

void ContactListModel::connectContact(const Contact &contact)
{
    connect(contact, SIGNAL(updated()), this, SLOT(contactUpdated()));
}

void ContactListModel::disconnectContact(const Contact &contact)
{
    disconnect(contact, 0, this, 0);
}

void ContactListModel::setContactList(const QVector<Contact> &contacts)
{
    beginResetModel();

    for (auto const &contact : m_list)
        disconnectContact(contact);
    m_list = contacts;
    for (auto const &contact : m_list)
        connectContact(contact);

    endResetModel();
}

void ContactListModel::addContact(const Contact &contact)
{
    if (m_list.contains(contact))
        return;

    connectContact(contact);

    beginInsertRows(QModelIndex(), m_list.count(), m_list.count());
    m_list.append(contact);
    endInsertRows();
}

void ContactListModel::removeContact(const Contact &contact)
{
    int index = m_list.indexOf(contact);
    if (-1 == index)
        return;

    disconnectContact(contact);

    beginRemoveRows(QModelIndex(), index, index);
    m_list.remove(index);
    endRemoveRows();
}

void ContactListModel::contactUpdated()
{
    ContactShared *contactShared = qobject_cast<ContactShared *>(sender());
    if (!contactShared)
        return;

    int row = m_list.indexOf(Contact(contactShared));
    if (row < 0)
        return;

    const QModelIndex &contactIndex = index(row, 0);
    emit dataChanged(contactIndex, contactIndex);
}

QModelIndex ContactListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || !hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, m_list.at(row).data());
}

QModelIndex ContactListModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)

    return QModelIndex();
}

int ContactListModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

int ContactListModel::rowCount(const QModelIndex &parentIndex) const
{
    return parentIndex.isValid() ? 0 : m_list.size();
}

QVariant ContactListModel::data(const QModelIndex &index, int role) const
{
    QObject *sharedData = static_cast<QObject *>(index.internalPointer());
    Q_ASSERT(sharedData);

    ContactShared *contact = qobject_cast<ContactShared *>(sharedData);
    if (!contact)
        return QVariant();

    return m_contactDataExtractor->data(Contact(contact), role, true);
}

QModelIndexList ContactListModel::indexListForValue(const QVariant &value) const
{
    QModelIndexList result;

    const Buddy &buddy = value.value<Buddy>();

    const int size = m_list.size();
    for (int i = 0; i < size; i++)
    {
        const Contact &contact = m_list.at(i);
        if (contact.ownerBuddy() == buddy)
            result.append(index(i, 0));
    }

    return result;
}
