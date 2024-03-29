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

#include <QtCore/QFutureWatcher>
#include <QtWidgets/QAction>

#include "buddies/model/buddy-list-model.h"
#include "chat/model/chat-list-model.h"
#include "chats-buddies-splitter.h"
#include "model/action-list-model.h"
#include "model/merged-proxy-model-factory.h"
#include "plugin/plugin-injected-factory.h"

#include "history-talkable-combo-box.h"
#include "moc_history-talkable-combo-box.cpp"

HistoryTalkableComboBox::HistoryTalkableComboBox(QWidget *parent)
        : SelectTalkableComboBox(parent), TalkablesFutureWatcher(0)
{
}

HistoryTalkableComboBox::~HistoryTalkableComboBox()
{
}

void HistoryTalkableComboBox::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void HistoryTalkableComboBox::init()
{
    setShowAnonymous(true);

    ActionListModel *actionModel = new ActionListModel(this);
    AllAction = new QAction(this);
    actionModel->appendAction(AllAction);

    ChatsModel = m_pluginInjectedFactory->makeInjected<ChatListModel>(this);
    BuddiesModel = m_pluginInjectedFactory->makeInjected<BuddyListModel>(this);

    QList<KaduAbstractModel *> models;
    models.append(actionModel);
    models.append(ChatsModel);
    models.append(BuddiesModel);

    setBaseModel(MergedProxyModelFactory::createKaduModelInstance(models, this));
}

void HistoryTalkableComboBox::setAllLabel(const QString &allLabel)
{
    AllAction->setText(allLabel);
}

void HistoryTalkableComboBox::setTalkables(const QVector<Talkable> &talkables)
{
    futureTalkablesCanceled();

    auto chatsBuddies = m_pluginInjectedFactory->makeUnique<ChatsBuddiesSplitter>(talkables);

    ChatsModel->setChats(chatsBuddies->chats().toList().toVector());
    BuddiesModel->setBuddyList(chatsBuddies->buddies().toList());
}

void HistoryTalkableComboBox::setFutureTalkables(const QFuture<QVector<Talkable>> &talkables)
{
    if (TalkablesFutureWatcher)
        delete TalkablesFutureWatcher;

    ChatsModel->setChats(QVector<Chat>());
    BuddiesModel->setBuddyList(BuddyList());

    TalkablesFutureWatcher = new QFutureWatcher<QVector<Talkable>>(this);
    connect(TalkablesFutureWatcher, SIGNAL(finished()), this, SLOT(futureTalkablesAvailable()));
    connect(TalkablesFutureWatcher, SIGNAL(canceled()), this, SLOT(futureTalkablesCanceled()));
    TalkablesFutureWatcher->setFuture(talkables);
}

void HistoryTalkableComboBox::futureTalkablesAvailable()
{
    if (TalkablesFutureWatcher)
        setTalkables(TalkablesFutureWatcher->result());
}

void HistoryTalkableComboBox::futureTalkablesCanceled()
{
    if (!TalkablesFutureWatcher)
        return;

    TalkablesFutureWatcher->deleteLater();
    TalkablesFutureWatcher = 0;
}
