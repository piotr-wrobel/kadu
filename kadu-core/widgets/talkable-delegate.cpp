/*
 * %kadu copyright begin%
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "talkable-delegate.h"
#include "moc_talkable-delegate.cpp"

#include "accounts/account.h"
#include "buddies/buddy-preferred-manager.h"
#include "contacts/contact-manager.h"
#include "message/unread-message-repository.h"
#include "model/kadu-abstract-model.h"
#include "model/model-chain.h"
#include "widgets/talkable-painter.h"

#include <QtGui/QKeyEvent>
#include <QtWidgets/QAbstractItemView>

TalkableDelegate::TalkableDelegate(TalkableTreeView *parent) : KaduTreeViewDelegate(parent)
{
}

TalkableDelegate::~TalkableDelegate()
{
}

void TalkableDelegate::setBuddyPreferredManager(BuddyPreferredManager *buddyPreferredManager)
{
    m_buddyPreferredManager = buddyPreferredManager;
}

void TalkableDelegate::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

void TalkableDelegate::setUnreadMessageRepository(UnreadMessageRepository *unreadMessageRepository)
{
    m_unreadMessageRepository = unreadMessageRepository;
}

void TalkableDelegate::init()
{
    connect(m_contactManager, SIGNAL(contactUpdated(Contact)), this, SLOT(contactUpdated(Contact)));
    connect(m_buddyPreferredManager, SIGNAL(buddyUpdated(Buddy)), this, SLOT(buddyUpdated(Buddy)));
    connect(m_unreadMessageRepository, SIGNAL(unreadMessageAdded(Message)), this, SLOT(messageStatusChanged(Message)));
    connect(
        m_unreadMessageRepository, SIGNAL(unreadMessageRemoved(Message)), this, SLOT(messageStatusChanged(Message)));
}

void TalkableDelegate::setChain(ModelChain *chain)
{
    Chain = chain;
}

void TalkableDelegate::contactUpdated(const Contact &contact)
{
    if (!Chain)
        return;

    const QModelIndexList &contactsIndexList = Chain.data()->indexListForValue(contact);
    for (auto const &contactIndex : contactsIndexList)
        emit sizeHintChanged(contactIndex);
}

void TalkableDelegate::buddyUpdated(const Buddy &buddy)
{
    if (!Chain)
        return;

    const QModelIndexList &buddyIndexList = Chain.data()->indexListForValue(buddy);
    for (auto const &buddyIndex : buddyIndexList)
        emit sizeHintChanged(buddyIndex);
}

void TalkableDelegate::messageStatusChanged(Message message)
{
    Buddy buddy = message.messageSender().ownerBuddy();
    buddyUpdated(buddy);
}

bool TalkableDelegate::editorEvent(
    QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!event || !model)
        return false;

    Qt::ItemFlags flags = model->flags(index);
    if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled) || !(flags & Qt::ItemIsEnabled))
        return false;

    QVariant value = index.data(Qt::CheckStateRole);
    if (!value.isValid())
        return false;

    switch (event->type())
    {
    case QEvent::MouseButtonRelease:
    {
        TalkablePainter talkablePainter(configuration(), getOptions(index, option), index);

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (Qt::LeftButton != mouseEvent->button())
            return false;

        const QRect &checkboxRect = talkablePainter.checkboxRect();
        if (!checkboxRect.contains(mouseEvent->pos()))
            return false;

        break;
    }
    case QEvent::MouseButtonDblClick:
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (Qt::LeftButton != mouseEvent->button())
            return false;

        break;
    }

    case QEvent::KeyPress:
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (Qt::Key_Space != keyEvent->key() && Qt::Key_Select != keyEvent->key())
            return false;

        break;
    }

    default:
        return false;
    }

    Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
    state = state == Qt::Checked ? Qt::Unchecked : Qt::Checked;

    return model->setData(index, state, Qt::CheckStateRole);
}
