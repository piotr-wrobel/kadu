/*
 * %kadu copyright begin%
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "chat-history-tab.h"
#include "moc_chat-history-tab.cpp"

#include "gui/widgets/timeline-chat-messages-view.h"
#include "storage/history-messages-storage.h"

#include "contacts/contact-set.h"
#include "model/model-chain.h"
#include "talkable/talkable-converter.h"
#include "widgets/talkable-tree-view.h"

#include <QtCore/QModelIndexList>
#include <QtWidgets/QAbstractItemView>

ChatHistoryTab::ChatHistoryTab(QWidget *parent) : HistoryMessagesTab(parent)
{
}

ChatHistoryTab::~ChatHistoryTab()
{
}

void ChatHistoryTab::setTalkableConverter(TalkableConverter *talkableConverter)
{
    m_talkableConverter = talkableConverter;
}

void ChatHistoryTab::init()
{
    timelineView()->setTalkableVisible(false);
    timelineView()->setTitleVisible(true);

    setClearHistoryMenuItemTitle(tr("&Clear Chat History"));
}

void ChatHistoryTab::talkablesAvailable()
{
    if (!m_talkableToSelect.isValidChat())
        return;

    QModelIndexList indexesToSelect;

    auto chat = m_talkableConverter->toChat(m_talkableToSelect);
    m_talkableToSelect = Talkable();

    if (chat.contacts().size() == 1)
        indexesToSelect = modelChain()->indexListForValue(chat.contacts().begin()->ownerBuddy());
    else if (chat.contacts().size() > 1)
        indexesToSelect = modelChain()->indexListForValue(chat);

    if (1 == indexesToSelect.size())
    {
        talkableTree()->selectionModel()->select(indexesToSelect.at(0), QItemSelectionModel::ClearAndSelect);
        talkableTree()->scrollTo(indexesToSelect.at(0), QAbstractItemView::EnsureVisible);
        displayTalkable(chat, false);
    }
    else
        talkableTree()->selectionModel()->select(QModelIndex(), QItemSelectionModel::ClearAndSelect);
}

void ChatHistoryTab::selectTalkable(const Talkable &talkable)
{
    m_talkableToSelect = talkable;
}
