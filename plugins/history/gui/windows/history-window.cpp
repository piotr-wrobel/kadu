/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009, 2009, 2009, 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010, 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2010, 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2008, 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
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
#include <QtGui/QDateEdit>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QItemDelegate>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>

#include "accounts/account-manager.h"
#include "buddies/buddy-manager.h"
#include "buddies/model/buddies-model-base.h"
#include "buddies/model/buddy-list-model.h"
#include "chat/aggregate-chat-manager.h"
#include "chat/chat-details-aggregate.h"
#include "chat/chat-manager.h"
#include "chat/model/chats-list-model.h"
#include "chat/type/chat-type-manager.h"
#include "chat/type/chat-type.h"
#include "gui/actions/actions.h"
#include "gui/actions/base-action-context.h"
#include "gui/widgets/chat-widget-manager.h"
#include "gui/widgets/chat-widget.h"
#include "gui/widgets/delayed-line-edit.h"
#include "gui/widgets/filter-widget.h"
#include "gui/widgets/filtered-tree-view.h"
#include "gui/widgets/talkable-delegate-configuration.h"
#include "gui/widgets/talkable-menu-manager.h"
#include "gui/widgets/talkable-tree-view.h"
#include "gui/windows/message-dialog.h"
#include "icons/kadu-icon.h"
#include "message/message.h"
#include "misc/misc.h"
#include "model/merged-proxy-model-factory.h"
#include "model/model-chain.h"
#include "model/roles.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol-menu-manager.h"
#include "status/status-type-data.h"
#include "status/status-type-manager.h"
#include "talkable/filter/name-talkable-filter.h"
#include "talkable/model/talkable-proxy-model.h"
#include "activate.h"
#include "debug.h"

#include "model/dates-model-item.h"
#include "model/history-dates-model.h"
#include "gui/widgets/chat-history-tab.h"
#include "gui/widgets/timeline-chat-messages-view.h"
#include "search/history-search-parameters.h"
#include "storage/history-storage.h"
#include "history.h"
#include "timed-status.h"

#include "history-window.h"

HistoryWindow * HistoryWindow::Instance = 0;

void HistoryWindow::show(const Chat &chat)
{
	if (!History::instance()->currentStorage())
	{
		MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("There is no history storage plugin loaded!"));
		return;
	}

	Chat aggregate = AggregateChatManager::instance()->aggregateChat(chat);
	if (!aggregate)
		aggregate = chat;

	if (!Instance)
		Instance = new HistoryWindow();

	Instance->updateData();
	Instance->selectChat(aggregate);

	Instance->setVisible(true);
	_activateWindow(Instance);
}

HistoryWindow::HistoryWindow(QWidget *parent) :
		QMainWindow(parent)
{
	kdebugf();

	setProperty("ownWindowIcon", true);
	setWindowRole("kadu-history");
	setAttribute(Qt::WA_DeleteOnClose);

	setWindowTitle(tr("History"));
	setWindowIcon(KaduIcon("kadu_icons/history").icon());

	createGui();

	loadWindowGeometry(this, "History", "HistoryWindowGeometry", 200, 200, 750, 500);

	StatusDetailsPopupMenu = new QMenu(this);
	StatusDetailsPopupMenu->addAction(KaduIcon("kadu_icons/clear-history").icon(), tr("&Remove entries"),
	                                  this, SLOT(removeStatusEntriesPerDate()));

	SmsDetailsPopupMenu = new QMenu(this);
	SmsDetailsPopupMenu->addAction(KaduIcon("kadu_icons/clear-history").icon(), tr("&Remove entries"),
	                               this, SLOT(removeSmstEntriesPerDate()));

	kdebugf2();
}

HistoryWindow::~HistoryWindow()
{
	kdebugf();

	saveWindowGeometry(this, "History", "HistoryDialogGeometry");

	Instance = 0;

	kdebugf2();
}

void HistoryWindow::createGui()
{
	QWidget *mainWidget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(mainWidget);
	layout->setMargin(0);
	layout->setSpacing(0);

	TabWidget = new QTabWidget(mainWidget);
	TabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	TabWidget->setDocumentMode(true);

	ChatTab = new ChatHistoryTab(TabWidget);
	TabWidget->addTab(ChatTab, tr("Chats"));
	TabWidget->addTab(createStatusTab(TabWidget), tr("Statuses"));
	TabWidget->addTab(createSmsTab(TabWidget), tr("SMS"));

	MyBuddyStatusDatesModel = new HistoryDatesModel(false, this);
	MySmsDatesModel = new HistoryDatesModel(false, this);

	QDialogButtonBox *buttons = new QDialogButtonBox(mainWidget);
	buttons->addButton(tr("Search in History..."), QDialogButtonBox::ActionRole);
	QPushButton *closeButton = buttons->addButton(QDialogButtonBox::Close);
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

	buttons->layout()->setMargin(5);

	layout->addWidget(TabWidget);
	layout->addWidget(buttons);

	setCentralWidget(mainWidget);
}

QWidget * HistoryWindow::createStatusTab(QWidget *parent)
{
	QSplitter *splitter = new QSplitter(Qt::Horizontal, parent);

	FilteredTreeView *statusesTalkableWidget = new FilteredTreeView(FilteredTreeView::FilterAtTop, splitter);
	statusesTalkableWidget->setFilterAutoVisibility(false);

	StatusesTalkableTree = new TalkableTreeView(statusesTalkableWidget);
	StatusesTalkableTree->setUseConfigurationColors(true);
	StatusesTalkableTree->setContextMenuEnabled(true);
	StatusesTalkableTree->delegateConfiguration().setShowMessagePixmap(false);

	StatusBuddiesModel = new BuddyListModel(StatusesTalkableTree);
	StatusesModelChain = new ModelChain(StatusBuddiesModel, StatusesTalkableTree);

	TalkableProxyModel *proxyModel = new TalkableProxyModel(StatusesModelChain);
	proxyModel->setSortByStatusAndUnreadMessages(false);

	NameTalkableFilter *nameTalkableFilter = new NameTalkableFilter(NameTalkableFilter::AcceptMatching, proxyModel);
	connect(statusesTalkableWidget, SIGNAL(filterChanged(QString)), nameTalkableFilter, SLOT(setName(QString)));
	proxyModel->addFilter(nameTalkableFilter);

	StatusesModelChain->addProxyModel(proxyModel);

	StatusesTalkableTree->setChain(StatusesModelChain);

	connect(StatusesTalkableTree, SIGNAL(currentChanged(Talkable)), this, SLOT(currentStatusChanged(Talkable)));
	connect(StatusesTalkableTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showStatusesPopupMenu(QPoint)));
	StatusesTalkableTree->setContextMenuPolicy(Qt::CustomContextMenu);

	statusesTalkableWidget->setView(StatusesTalkableTree);

	TimelineStatusesView = new TimelineChatMessagesView(splitter);

	TimelineStatusesView->timeline()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(TimelineStatusesView->timeline(), SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(showStatusDetailsPopupMenu(QPoint)));

	QList<int> sizes;
	sizes.append(150);
	sizes.append(300);
	splitter->setSizes(sizes);

	return splitter;
}

QWidget * HistoryWindow::createSmsTab(QWidget *parent)
{
	QSplitter *splitter = new QSplitter(Qt::Horizontal, parent);

	FilteredTreeView *smsListWidget = new FilteredTreeView(FilteredTreeView::FilterAtTop, splitter);
	smsListWidget->setFilterAutoVisibility(false);

	SmsListView = new KaduTreeView(smsListWidget);
	SmsListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	SmsListView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	SmsModel = new QStandardItemModel(StatusesTalkableTree);
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(SmsModel);
	proxyModel->setSourceModel(SmsModel);

	connect(smsListWidget, SIGNAL(filterChanged(QString)), proxyModel, SLOT(setFilterFixedString(QString)));

	SmsListView->setModel(proxyModel);

	connect(SmsListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
	        this, SLOT(currentSmsChanged(QModelIndex,QModelIndex)));
	connect(SmsListView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showSmsPopupMenu(QPoint)));
	SmsListView->setContextMenuPolicy(Qt::CustomContextMenu);

	smsListWidget->setView(SmsListView);

	TimelineSmsesView = new TimelineChatMessagesView(splitter);

	TimelineSmsesView->timeline()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(TimelineSmsesView->timeline(), SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(showSmsDetailsPopupMenu(QPoint)));

	QList<int> sizes;
	sizes.append(150);
	sizes.append(300);
	splitter->setSizes(sizes);

	return splitter;
}

void HistoryWindow::updateData()
{
	kdebugf();

	ChatTab->updateData();

	QVector<Buddy> statusBuddies = History::instance()->statusBuddiesList(HistorySearchParameters());
	StatusBuddiesModel->setBuddyList(statusBuddies.toList());

	QList<QString> smsRecipients = History::instance()->smsRecipientsList(HistorySearchParameters());

	SmsModel->clear();
	foreach (const QString &smsRecipient, smsRecipients)
		SmsModel->appendRow(new QStandardItem(KaduIcon("phone").icon(), smsRecipient));

	ChatTab->selectChat(Chat::null);
}

void HistoryWindow::selectChat(const Chat &chat)
{
	TabWidget->setCurrentIndex(0);
	ChatTab->selectChat(chat);
}

void HistoryWindow::statusBuddyActivated(const Buddy &buddy)
{
	kdebugf();

	QModelIndex selectedIndex = TimelineStatusesView->timeline()->model()
	        ? TimelineStatusesView->timeline()->selectionModel()->currentIndex()
	        : QModelIndex();

	QDate date = selectedIndex.data(DateRole).toDate();

	QVector<DatesModelItem> statusDates = History::instance()->datesForStatusBuddy(buddy, HistorySearchParameters());
	MyBuddyStatusDatesModel->setDates(statusDates);

	if (date.isValid())
		selectedIndex = MyBuddyStatusDatesModel->indexForDate(date);
	if (!selectedIndex.isValid())
	{
		int lastRow = MyBuddyStatusDatesModel->rowCount(QModelIndex()) - 1;
		if (lastRow >= 0)
			selectedIndex = MyBuddyStatusDatesModel->index(lastRow);
	}

	TimelineStatusesView->timeline()->setModel(MyBuddyStatusDatesModel);

	connect(TimelineStatusesView->timeline()->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
	        this, SLOT(statusDateCurrentChanged(QModelIndex,QModelIndex)), Qt::UniqueConnection);

	TimelineStatusesView->timeline()->selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

	kdebugf2();
}

void HistoryWindow::smsRecipientActivated(const QString& recipient)
{
	kdebugf();

	QModelIndex selectedIndex = TimelineSmsesView->timeline()->model()
	        ? TimelineSmsesView->timeline()->selectionModel()->currentIndex()
	        : QModelIndex();

	QDate date = selectedIndex.data(DateRole).toDate();

	QVector<DatesModelItem> smsDates = History::instance()->datesForSmsRecipient(recipient, HistorySearchParameters());
	MySmsDatesModel->setDates(smsDates);

	if (date.isValid())
		selectedIndex = MySmsDatesModel->indexForDate(date);
	if (!selectedIndex.isValid())
	{
		int lastRow = MySmsDatesModel->rowCount(QModelIndex()) - 1;
		if (lastRow >= 0)
			selectedIndex = MySmsDatesModel->index(lastRow);
	}

	TimelineSmsesView->timeline()->setModel(MySmsDatesModel);

	connect(TimelineSmsesView->timeline()->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
	        this, SLOT(smsDateCurrentChanged(QModelIndex,QModelIndex)), Qt::UniqueConnection);

	TimelineSmsesView->timeline()->selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

	kdebugf2();
}

void HistoryWindow::statusDateCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	kdebugf();

	if (current == previous)
		return;

	QDate date = current.data(DateRole).value<QDate>();

	TimelineStatusesView->messagesView()->setUpdatesEnabled(false);

	TimelineStatusesView->messagesView()->clearMessages();

	if (!StatusesTalkableTree->actionContext()->buddies().isEmpty())
	{
		Buddy buddy = *StatusesTalkableTree->actionContext()->buddies().begin();
		QList<TimedStatus> statuses;
		if (buddy && date.isValid())
			statuses = History::instance()->statuses(buddy, date);
		if (!buddy.contacts().isEmpty())
			TimelineStatusesView->messagesView()->setChat(ChatManager::instance()->findChat(ContactSet(buddy.contacts().at(0)), true));
		TimelineStatusesView->messagesView()->appendMessages(statusesToMessages(statuses));
	}

	TimelineStatusesView->messagesView()->setUpdatesEnabled(true);

	kdebugf2();
}

void HistoryWindow::smsDateCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	kdebugf();

	if (current == previous)
		return;

	QDate date = current.data(DateRole).value<QDate>();

	TimelineSmsesView->messagesView()->setUpdatesEnabled(false);

	QString recipient = SmsListView->currentIndex().data().toString();
	QVector<Message> sms;
	if (!recipient.isEmpty() && date.isValid())
		sms = History::instance()->sms(recipient, date);
	TimelineSmsesView->messagesView()->setChat(Chat::null);
	TimelineSmsesView->messagesView()->clearMessages();
	TimelineSmsesView->messagesView()->appendMessages(sms);

	TimelineSmsesView->messagesView()->setUpdatesEnabled(true);

	kdebugf2();
}

QVector<Message> HistoryWindow::statusesToMessages(const QList<TimedStatus> &statuses)
{
	QVector<Message> messages;

	foreach (const TimedStatus &timedStatus, statuses)
	{
		Message message = Message::create();
		message.setStatus(MessageStatusReceived);
		message.setType(MessageTypeReceived);

		const StatusTypeData &typeData = StatusTypeManager::instance()->statusTypeData(timedStatus.status().type());

		if (timedStatus.status().description().isEmpty())
			message.setContent(typeData.name());
		else
			message.setContent(QString("%1 with description: %2")
					.arg(typeData.name())
					.arg(timedStatus.status().description()));

		message.setReceiveDate(timedStatus.dateTime());
		message.setSendDate(timedStatus.dateTime());

		messages.append(message);
	}

	return messages;
}

void HistoryWindow::showStatusesPopupMenu(const QPoint &pos)
{
	Q_UNUSED(pos)

	QScopedPointer<QMenu> menu;

	menu.reset(TalkableMenuManager::instance()->menu(this, StatusesTalkableTree->actionContext()));
	menu->addSeparator();
	menu->addAction(KaduIcon("kadu_icons/clear-history").icon(),
			tr("&Clear Status History"), this, SLOT(clearStatusHistory()));

	menu->exec(QCursor::pos());
}

void HistoryWindow::showSmsPopupMenu(const QPoint &pos)
{
	Q_UNUSED(pos)

	QScopedPointer<QMenu> menu;

	menu.reset(new QMenu(this));
	menu->addSeparator();
	menu->addAction(KaduIcon("kadu_icons/clear-history").icon(),
			tr("&Clear SMS History"), this, SLOT(clearSmsHistory()));

	menu->exec(QCursor::pos());
}

void HistoryWindow::showStatusDetailsPopupMenu(const QPoint &pos)
{
	QDate date = TimelineStatusesView->timeline()->indexAt(pos).data(DateRole).value<QDate>();
	if (!date.isValid())
		return;

	if (!StatusesTalkableTree->actionContext()->buddies().isEmpty())
	{
		Buddy buddy = *StatusesTalkableTree->actionContext()->buddies().begin();
		if (buddy && !buddy.contacts().isEmpty())
			StatusDetailsPopupMenu->exec(QCursor::pos());
	}
}

void HistoryWindow::showSmsDetailsPopupMenu(const QPoint &pos)
{
	QDate date = TimelineSmsesView->timeline()->indexAt(pos).data(DateRole).value<QDate>();
	if (!date.isValid())
		return;

	if (!SmsListView->currentIndex().data().toString().isEmpty())
		SmsDetailsPopupMenu->exec(QCursor::pos());
}

void HistoryWindow::clearStatusHistory()
{
	if (!StatusesTalkableTree->actionContext())
		return;

	const BuddySet &buddies = StatusesTalkableTree->actionContext()->buddies();
	if (buddies.isEmpty())
		return;

	foreach (const Buddy &buddy, buddies)
		History::instance()->currentStorage()->clearStatusHistory(buddy);

	updateData();
}

void HistoryWindow::clearSmsHistory()
{
	bool removed = false;

	const QModelIndexList &indexes = SmsListView->selectionModel()->selectedIndexes();
	foreach (const QModelIndex &index, indexes)
	{
		QString recipient = index.data(Qt::DisplayRole).toString();
		if (recipient.isEmpty())
			continue;

		removed = true;
		History::instance()->currentStorage()->clearSmsHistory(recipient);
	}

	if (removed)
		updateData();
}

void HistoryWindow::removeStatusEntriesPerDate()
{
	QDate date = TimelineStatusesView->timeline()->currentIndex().data(DateRole).value<QDate>();
	if (!date.isValid())
		return;

	if (!StatusesTalkableTree->actionContext()->buddies().isEmpty())
	{
		Buddy buddy = *StatusesTalkableTree->actionContext()->buddies().begin();
		if (buddy && !buddy.contacts().isEmpty())
		{
			History::instance()->currentStorage()->clearStatusHistory(buddy, date);
			statusBuddyActivated(buddy);
		}
	}
}

void HistoryWindow::removeSmsEntriesPerDate()
{
	QDate date = TimelineSmsesView->timeline()->currentIndex().data(DateRole).value<QDate>();
	if (!date.isValid())
		return;

	if (!SmsListView->currentIndex().data().toString().isEmpty())
	{
		History::instance()->currentStorage()->clearSmsHistory(SmsListView->currentIndex().data().toString(), date);
		smsRecipientActivated(SmsListView->currentIndex().data().toString());
	}
}

void HistoryWindow::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape)
	{
		e->accept();
		close();
	}
	else
		QWidget::keyPressEvent(e);
}

void HistoryWindow::currentStatusChanged(const Talkable &talkable)
{
	statusBuddyActivated(talkable.toBuddy());
}

void HistoryWindow::currentSmsChanged(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous);

	smsRecipientActivated(current.data().toString());
}
