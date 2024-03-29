/*
 * %kadu copyright begin%
 * Copyright 2012 Marcel Zięba (marseel@gmail.com)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtCore/QScopedPointer>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>

#include "buddies/model/buddy-list-model.h"
#include "chat/buddy-chat-manager.h"
#include "chat/model/chat-list-model.h"
#include "gui/widgets/timeline-chat-messages-view.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "menu/menu-inventory.h"
#include "message/sorted-messages.h"
#include "model/merged-proxy-model-factory.h"
#include "model/model-chain.h"
#include "plugin/plugin-injected-factory.h"
#include "talkable/filter/hide-temporary-talkable-filter.h"
#include "talkable/filter/name-talkable-filter.h"
#include "talkable/model/talkable-proxy-model.h"
#include "talkable/talkable-converter.h"
#include "widgets/filter-widget.h"
#include "widgets/filtered-tree-view.h"
#include "widgets/search-bar.h"
#include "widgets/talkable-delegate-configuration.h"
#include "widgets/talkable-tree-view.h"
#include "widgets/wait-overlay.h"
#include "widgets/webkit-messages-view/webkit-messages-view.h"
#include "windows/message-dialog.h"

#include "chats-buddies-splitter.h"
#include "history-query-result.h"
#include "history-query.h"
#include "storage/history-messages-storage.h"

#include "history-messages-tab.h"
#include "moc_history-messages-tab.cpp"

HistoryMessagesTab::HistoryMessagesTab(QWidget *parent)
        : HistoryTab(parent), Storage(0), TabWaitOverlay(0), TalkablesFutureWatcher(0)
{
}

HistoryMessagesTab::~HistoryMessagesTab()
{
}

void HistoryMessagesTab::setBuddyChatManager(BuddyChatManager *buddyChatManager)
{
    m_buddyChatManager = buddyChatManager;
}

void HistoryMessagesTab::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void HistoryMessagesTab::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void HistoryMessagesTab::setMenuInventory(MenuInventory *menuInventory)
{
    m_menuInventory = menuInventory;
}

void HistoryMessagesTab::setTalkableConverter(TalkableConverter *talkableConverter)
{
    m_talkableConverter = talkableConverter;
}

void HistoryMessagesTab::init()
{
    createGui();
    createModelChain();
}

void HistoryMessagesTab::createGui()
{
    TimelinePopupMenu = new QMenu(this);
    TimelinePopupMenu->addAction(
        m_iconsManager->iconByPath(KaduIcon("kadu_icons/clear-history")), tr("&Remove entries"), this,
        SLOT(removeEntries()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(2);

    Splitter = new QSplitter(Qt::Horizontal, this);

    FilteredView = m_pluginInjectedFactory->makeInjected<FilteredTreeView>(FilteredTreeView::FilterAtTop, Splitter);
    FilteredView->filterWidget()->setAutoVisibility(false);
    FilteredView->filterWidget()->setLabel(tr("Filter") + ":");

    TalkableTree = m_pluginInjectedFactory->makeInjected<TalkableTreeView>(FilteredView);
    TalkableTree->setAlternatingRowColors(true);
    TalkableTree->setContextMenuEnabled(true);
    TalkableTree->setContextMenuPolicy(Qt::CustomContextMenu);
    TalkableTree->setUseConfigurationColors(true);
    TalkableTree->delegateConfiguration()->setShowMessagePixmap(false);

    QString style;
    style.append("QTreeView::branch:has-siblings:!adjoins-item { border-image: none; image: none }");
    style.append("QTreeView::branch:has-siblings:adjoins-item { border-image: none; image: none }");
    style.append("QTreeView::branch:has-childres:!has-siblings:adjoins-item { border-image: none; image: none }");
    TalkableTree->setStyleSheet(style);
    TalkableTree->viewport()->setStyleSheet(style);

    connect(TalkableTree, SIGNAL(currentChanged(Talkable)), this, SLOT(currentTalkableChanged(Talkable)));
    connect(TalkableTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTalkablePopupMenu()));

    FilteredView->setView(TalkableTree);

    TimelineView = m_pluginInjectedFactory->makeInjected<TimelineChatMessagesView>(Splitter);
    TimelineView->searchBar()->setAutoVisibility(false);
    TimelineView->searchBar()->setSearchWidget(this);
    TimelineView->timeline()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(TimelineView->timeline(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTimelinePopupMenu()));
    connect(timelineView(), SIGNAL(currentDateChanged()), this, SLOT(currentDateChanged()));

    QList<int> sizes;
    sizes.append(150);
    sizes.append(300);
    Splitter->setSizes(sizes);

    layout->addWidget(Splitter);

    setFocusProxy(FilteredView->filterWidget());
}

void HistoryMessagesTab::createModelChain()
{
    ChatsModel = m_pluginInjectedFactory->makeInjected<ChatListModel>(TalkableTree);
    BuddiesModel = m_pluginInjectedFactory->makeInjected<BuddyListModel>(TalkableTree);

    QList<KaduAbstractModel *> models;
    models.append(ChatsModel);
    models.append(BuddiesModel);

    Chain = new ModelChain(TalkableTree);
    Chain->setBaseModel(MergedProxyModelFactory::createKaduModelInstance(models, Chain));

    TalkableProxyModel *proxyModel = m_pluginInjectedFactory->makeInjected<TalkableProxyModel>(Chain);
    proxyModel->setSortByStatusAndUnreadMessages(false);

    proxyModel->addFilter(new HideTemporaryTalkableFilter(proxyModel));
    NameTalkableFilter *nameTalkableFilter = new NameTalkableFilter(NameTalkableFilter::AcceptMatching, proxyModel);
    connect(FilteredView, SIGNAL(filterChanged(QString)), nameTalkableFilter, SLOT(setName(QString)));
    proxyModel->addFilter(nameTalkableFilter);

    Chain->addProxyModel(proxyModel);

    TalkableTree->setChain(Chain);
}

void HistoryMessagesTab::displayTalkable(const Talkable &talkable, bool force)
{
    if (!force && CurrentTalkable == talkable)
        return;

    CurrentTalkable = talkable;
    auto chat = m_talkableConverter->toChat(CurrentTalkable);
    // if buddy do not have any contact we have to create chat manually
    if (!chat)
        chat = m_buddyChatManager->buddyChat(m_talkableConverter->toBuddy(CurrentTalkable));

    TimelineView->messagesView()->setChat(chat);

    HistoryQuery query;
    query.setTalkable(CurrentTalkable);

    if (Storage && !CurrentTalkable.isEmpty())
        TimelineView->setFutureResults(Storage->dates(query));
    else
        TimelineView->setResults(QVector<HistoryQueryResult>());
}

FilteredTreeView *HistoryMessagesTab::filteredView() const
{
    return FilteredView;
}

TalkableTreeView *HistoryMessagesTab::talkableTree() const
{
    return TalkableTree;
}

ModelChain *HistoryMessagesTab::modelChain() const
{
    return Chain;
}

void HistoryMessagesTab::showTabWaitOverlay()
{
    if (!TabWaitOverlay)
        TabWaitOverlay = m_pluginInjectedFactory->makeInjected<WaitOverlay>(this);
    else
        TabWaitOverlay->show();
}

void HistoryMessagesTab::hideTabWaitOverlay()
{
    TabWaitOverlay->deleteLater();
    TabWaitOverlay = 0;
}

void HistoryMessagesTab::talkablesAvailable()
{
}

TimelineChatMessagesView *HistoryMessagesTab::timelineView() const
{
    return TimelineView;
}

void HistoryMessagesTab::setTalkables(const QVector<Talkable> &talkables)
{
    auto chatsBuddies = m_pluginInjectedFactory->makeUnique<ChatsBuddiesSplitter>(talkables);

    ChatsModel->setChats(chatsBuddies->chats().toList().toVector());
    BuddiesModel->setBuddyList(chatsBuddies->buddies().toList());
}

void HistoryMessagesTab::futureTalkablesAvailable()
{
    hideTabWaitOverlay();

    if (!TalkablesFutureWatcher)
        return;

    setTalkables(TalkablesFutureWatcher->result());

    TalkablesFutureWatcher->deleteLater();
    TalkablesFutureWatcher = 0;

    talkablesAvailable();
}

void HistoryMessagesTab::futureTalkablesCanceled()
{
    hideTabWaitOverlay();

    if (!TalkablesFutureWatcher)
        return;

    TalkablesFutureWatcher->deleteLater();
    TalkablesFutureWatcher = 0;
}

void HistoryMessagesTab::setFutureTalkables(const QFuture<QVector<Talkable>> &futureTalkables)
{
    if (TalkablesFutureWatcher)
    {
        TalkablesFutureWatcher->cancel();
        TalkablesFutureWatcher->deleteLater();
    }

    TalkablesFutureWatcher = new QFutureWatcher<QVector<Talkable>>(this);
    connect(TalkablesFutureWatcher, SIGNAL(finished()), this, SLOT(futureTalkablesAvailable()));
    connect(TalkablesFutureWatcher, SIGNAL(canceled()), this, SLOT(futureTalkablesCanceled()));

    TalkablesFutureWatcher->setFuture(futureTalkables);

    showTabWaitOverlay();
}

void HistoryMessagesTab::currentTalkableChanged(const Talkable &talkable)
{
    displayTalkable(talkable, false);
}

void HistoryMessagesTab::currentDateChanged()
{
    QDate date = timelineView()->currentDate();

    if (!Storage || !date.isValid())
    {
        TimelineView->setMessages(SortedMessages());
        return;
    }

    HistoryQuery query;
    query.setTalkable(CurrentTalkable);
    query.setFromDate(date);
    query.setToDate(date);

    auto chat = m_talkableConverter->toChat(CurrentTalkable);
    // if buddy do not have any contact we have to create chat manually
    if (!chat)
        chat = m_buddyChatManager->buddyChat(m_talkableConverter->toBuddy(CurrentTalkable));

    timelineView()->messagesView()->setChat(chat);
    TimelineView->setFutureMessages(Storage->messages(query));
}

void HistoryMessagesTab::setClearHistoryMenuItemTitle(const QString &clearHistoryMenuItemTitle)
{
    ClearHistoryMenuItemTitle = clearHistoryMenuItemTitle;
}

void HistoryMessagesTab::showTalkablePopupMenu()
{
    QScopedPointer<QMenu> menu(new QMenu());
    m_menuInventory->menu("buddy-list")->attachToMenu(menu.data());
    m_menuInventory->menu("buddy-list")->applyTo(menu.data(), TalkableTree->actionContext());

    menu->addSeparator();
    menu->addAction(
        m_iconsManager->iconByPath(KaduIcon("kadu_icons/clear-history")), ClearHistoryMenuItemTitle, this,
        SLOT(clearTalkableHistory()));

    menu->exec(QCursor::pos());
}

void HistoryMessagesTab::clearTalkableHistory()
{
    if (!Storage)
        return;

    Q_ASSERT(TalkableTree->selectionModel());

    const QModelIndexList &selectedIndexes = TalkableTree->selectionModel()->selectedIndexes();
    QList<Talkable> talkables;

    MessageDialog *dialog = MessageDialog::create(
        m_iconsManager->iconByPath(KaduIcon("dialog-question")), tr("Kadu"),
        tr("Do you really want to delete history?"));
    dialog->addButton(QMessageBox::Yes, tr("Delete history"));
    dialog->addButton(QMessageBox::No, tr("Cancel"));

    if (!dialog->ask())
        return;

    for (auto const &selectedIndex : selectedIndexes)
    {
        Talkable talkable = selectedIndex.data(TalkableRole).value<Talkable>();
        if (!talkable.isEmpty())
            Storage->deleteMessages(talkable);
    }

    updateData();
    displayTalkable(Talkable(), true);
}

void HistoryMessagesTab::showTimelinePopupMenu()
{
    if (TimelineView->currentDate().isValid())
        TimelinePopupMenu->exec(QCursor::pos());
}

void HistoryMessagesTab::removeEntries()
{
    QDate date = TimelineView->currentDate();
    if (!Storage || !date.isValid())
        return;

    Storage->deleteMessages(CurrentTalkable, date);
    displayTalkable(CurrentTalkable, true);
}

void HistoryMessagesTab::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == QKeySequence::Copy && !TimelineView->messagesView()->selectedText().isEmpty())
        // Do not use triggerPageAction(), see bug #2345.
        TimelineView->messagesView()->pageAction(QWebPage::Copy)->trigger();
    else
        QWidget::keyPressEvent(event);
}

void HistoryMessagesTab::updateData()
{
    if (!Storage)
    {
        setTalkables(QVector<Talkable>());
        displayTalkable(Talkable(), false);
        return;
    }

    setFutureTalkables(Storage->talkables());
}

QList<int> HistoryMessagesTab::sizes() const
{
    QList<int> result = Splitter->sizes();
    result.append(TimelineView->sizes());

    return result;
}

void HistoryMessagesTab::setSizes(const QList<int> &newSizes)
{
    Q_ASSERT(newSizes.size() == 4);

    Splitter->setSizes(newSizes.mid(0, 2));
    TimelineView->setSizes(newSizes.mid(2, 2));
}

void HistoryMessagesTab::setHistoryMessagesStorage(HistoryMessagesStorage *storage)
{
    Storage = storage;
    updateData();
}

HistoryMessagesStorage *HistoryMessagesTab::historyMessagesStorage() const
{
    return Storage;
}
