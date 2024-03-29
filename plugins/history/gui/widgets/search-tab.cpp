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

#include <QtCore/QVariant>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>

#include "activate.h"
#include "chat/chat-storage.h"
#include "core/core.h"
#include "gui/web-view-highlighter.h"
#include "icons/kadu-icon.h"
#include "message/sorted-messages.h"
#include "misc/misc.h"
#include "model/roles.h"
#include "plugin/plugin-injected-factory.h"
#include "widgets/search-bar.h"
#include "widgets/webkit-messages-view/webkit-messages-view.h"

#include "gui/widgets/history-talkable-combo-box.h"
#include "gui/widgets/timeline-chat-messages-view.h"
#include "history-query.h"
#include "storage/history-messages-storage.h"
#include "talkable/talkable-converter.h"

#include "search-tab.h"
#include "moc_search-tab.cpp"

SearchTab::SearchTab(QWidget *parent)
        : HistoryTab(parent), m_historyChatStorage(0), StatusStorage(0), SmsStorage(0),
          SearchedStorage(&m_historyChatStorage)
{
}

SearchTab::~SearchTab()
{
}

void SearchTab::setChatStorage(ChatStorage *chatStorage)
{
    m_chatStorage = chatStorage;
}

void SearchTab::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void SearchTab::setTalkableConverter(TalkableConverter *talkableConverter)
{
    m_talkableConverter = talkableConverter;
}

void SearchTab::init()
{
    createGui();
}

void SearchTab::createGui()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(2);
    layout->setSpacing(0);

    Splitter = new QSplitter(Qt::Horizontal, this);
    layout->addWidget(Splitter);

    QWidget *queryWidget = new QWidget(Splitter);
    QVBoxLayout *queryLayout = new QVBoxLayout(queryWidget);
    queryLayout->setMargin(3);

    QWidget *queryFormWidget = new QWidget(queryWidget);
    queryLayout->addWidget(queryFormWidget);

    QFormLayout *queryFormLayout = new QFormLayout(queryFormWidget);
    queryFormLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    queryFormLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    queryFormLayout->setMargin(0);

    Query = new QLineEdit(queryFormWidget);
    Query->setMinimumWidth(200);
    queryFormLayout->addRow(tr("Search for:"), Query);

    connect(Query, SIGNAL(returnPressed()), this, SLOT(performSearch()));

    SearchInChats = new QRadioButton(tr("Chats"), queryFormWidget);
    SearchInChats->setChecked(true);
    SelectChat = m_pluginInjectedFactory->makeInjected<HistoryTalkableComboBox>(queryFormWidget);
    SelectChat->setAllLabel(tr(" - All chats - "));
    SelectChat->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    queryFormLayout->addRow(SearchInChats, SelectChat);

    SearchInStatuses = new QRadioButton(tr("Statuses"), queryFormWidget);
    SelectStatusBuddy = m_pluginInjectedFactory->makeInjected<HistoryTalkableComboBox>(queryFormWidget);
    SelectStatusBuddy->setAllLabel(tr(" - All buddies - "));
    SelectStatusBuddy->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    SelectStatusBuddy->setEnabled(false);
    queryFormLayout->addRow(SearchInStatuses, SelectStatusBuddy);

    SearchInSmses = new QRadioButton(tr("Smses"), queryFormWidget);
    SelectSmsRecipient = m_pluginInjectedFactory->makeInjected<HistoryTalkableComboBox>(queryFormWidget);
    SelectSmsRecipient->setAllLabel(tr(" - All recipients - "));
    SelectSmsRecipient->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    SelectSmsRecipient->setEnabled(false);
    queryFormLayout->addRow(SearchInSmses, SelectSmsRecipient);

    QButtonGroup *kindRadioGroup = new QButtonGroup(queryFormWidget);
    kindRadioGroup->addButton(SearchInChats);
    kindRadioGroup->addButton(SearchInStatuses);
    kindRadioGroup->addButton(SearchInSmses);
    connect(kindRadioGroup, SIGNAL(buttonReleased(QAbstractButton *)), this, SLOT(kindChanged(QAbstractButton *)));

    SearchByDate = new QCheckBox(tr("By date"), queryFormWidget);
    SearchByDate->setCheckState(Qt::Unchecked);

    QWidget *dateWidget = new QWidget(queryFormWidget);

    QHBoxLayout *dateLayout = new QHBoxLayout(dateWidget);

    FromDate = new QDateEdit(dateWidget);
    FromDate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    FromDate->setCalendarPopup(true);
    FromDate->setDate(QDate::currentDate().addDays(-7));
    dateLayout->addWidget(FromDate);

    dateLayout->addWidget(new QLabel(tr("to"), dateWidget));

    ToDate = new QDateEdit(dateWidget);
    ToDate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ToDate->setCalendarPopup(true);
    ToDate->setDate(QDate::currentDate());
    dateLayout->addWidget(ToDate);

    connect(FromDate, SIGNAL(dateChanged(QDate)), this, SLOT(fromDateChanged(QDate)));
    connect(ToDate, SIGNAL(dateChanged(QDate)), this, SLOT(toDateChanged(QDate)));
    connect(SearchByDate, SIGNAL(toggled(bool)), dateWidget, SLOT(setEnabled(bool)));

    dateWidget->setEnabled(false);
    queryFormLayout->addRow(SearchByDate, dateWidget);

    QPushButton *searchButton = new QPushButton(tr("Search"), queryFormWidget);
    searchButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    queryFormLayout->addRow(0, searchButton);

    connect(searchButton, SIGNAL(clicked()), this, SLOT(performSearch()));

    TimelineView = m_pluginInjectedFactory->makeInjected<TimelineChatMessagesView>(Splitter);
    TimelineView->setTalkableVisible(true);
    TimelineView->setTitleVisible(true);
    TimelineView->setLengthHeader(tr("Found"));
    connect(TimelineView, SIGNAL(currentDateChanged()), this, SLOT(currentDateChanged()));
    connect(TimelineView, SIGNAL(messagesDisplayed()), this, SLOT(messagesDisplayed()));

    TimelineView->searchBar()->setAutoVisibility(false);
    TimelineView->searchBar()->setSearchWidget(this);
    connect(TimelineView->searchBar(), SIGNAL(clearSearch()), this, SLOT(clearSelect()));

    setFocusProxy(Query);
}

void SearchTab::setHistoryChatStorage(HistoryMessagesStorage *storage)
{
    if (m_historyChatStorage == storage)
        return;

    m_historyChatStorage = storage;

    if (!m_historyChatStorage)
        SelectChat->setTalkables(QVector<Talkable>());
    else
        SelectChat->setFutureTalkables(m_historyChatStorage->talkables());

    if (*SearchedStorage == m_historyChatStorage)
    {
        TimelineView->setResults(QVector<HistoryQueryResult>());
        TimelineView->messagesView()->clearMessages();
        TimelineView->messagesView()->setChat(Chat::null);
    }
}

void SearchTab::setStatusStorage(HistoryMessagesStorage *storage)
{
    if (StatusStorage == storage)
        return;

    StatusStorage = storage;

    if (!StatusStorage)
        SelectStatusBuddy->setTalkables(QVector<Talkable>());
    else
        SelectStatusBuddy->setFutureTalkables(StatusStorage->talkables());

    if (*SearchedStorage == StatusStorage)
    {
        TimelineView->setResults(QVector<HistoryQueryResult>());
        TimelineView->messagesView()->clearMessages();
        TimelineView->messagesView()->setChat(Chat::null);
    }
}

void SearchTab::setSmsStorage(HistoryMessagesStorage *storage)
{
    if (SmsStorage == storage)
        return;

    SmsStorage = storage;

    if (!SmsStorage)
        SelectSmsRecipient->setTalkables(QVector<Talkable>());
    else
        SelectSmsRecipient->setFutureTalkables(SmsStorage->talkables());

    if (*SearchedStorage == SmsStorage)
    {
        TimelineView->setResults(QVector<HistoryQueryResult>());
        TimelineView->messagesView()->clearMessages();
        TimelineView->messagesView()->setChat(Chat::null);
    }
}

void SearchTab::kindChanged(QAbstractButton *button)
{
    SelectChat->setEnabled(SearchInChats == button);
    SelectStatusBuddy->setEnabled(SearchInStatuses == button);
    SelectSmsRecipient->setEnabled(SearchInSmses == button);
}

void SearchTab::fromDateChanged(const QDate &date)
{
    if (ToDate->date() < date)
        ToDate->setDate(date);
}

void SearchTab::toDateChanged(const QDate &date)
{
    if (FromDate->date() > date)
        FromDate->setDate(date);
}

void SearchTab::performSearch()
{
    HistoryQuery query;
    query.setString(Query->text());

    if (SearchByDate->isChecked())
    {
        query.setFromDate(FromDate->date());
        query.setToDate(ToDate->date());
    }

    if (SearchInChats->isChecked())
    {
        query.setTalkable(SelectChat->currentTalkable());
        SearchedStorage = &m_historyChatStorage;
        TimelineView->setTalkableHeader(tr("Chat"));
    }
    else if (SearchInStatuses->isChecked())
    {
        query.setTalkable(SelectStatusBuddy->currentTalkable());
        SearchedStorage = &StatusStorage;
        TimelineView->setTalkableHeader(tr("Buddy"));
    }
    else if (SearchInSmses->isChecked())
    {
        query.setTalkable(SelectSmsRecipient->currentTalkable());
        SearchedStorage = &SmsStorage;
        TimelineView->setTalkableHeader(tr("Recipient"));
    }

    if (SearchedStorage && *SearchedStorage)
        TimelineView->setFutureResults((*SearchedStorage)->dates(query));
    else
        TimelineView->setResults(QVector<HistoryQueryResult>());
}

void SearchTab::clearSelect()
{
    TimelineView->highlighter()->setHighlight(Query->text());
    TimelineView->highlighter()->selectNext(Query->text());
}

void SearchTab::currentDateChanged()
{
    const QModelIndex &currentIndex = TimelineView->timeline()->currentIndex();
    if (!currentIndex.isValid())
    {
        TimelineView->messagesView()->setChat(Chat::null);
        TimelineView->messagesView()->clearMessages();
        return;
    }

    const Talkable talkable = currentIndex.data(TalkableRole).value<Talkable>();
    const QDate date = currentIndex.data(DateRole).value<QDate>();

    auto chat = m_talkableConverter->toChat(talkable);
    if (!chat)
    {
        chat = m_chatStorage->create("");
        chat.setDisplay("?");
    }
    TimelineView->messagesView()->setChat(chat);

    if (SearchedStorage && *SearchedStorage)
    {
        HistoryQuery query;
        query.setTalkable(talkable);
        query.setFromDate(date);
        query.setToDate(date);

        TimelineView->setFutureMessages((*SearchedStorage)->messages(query));
    }
    else
        TimelineView->setMessages(SortedMessages());
}

void SearchTab::messagesDisplayed()
{
    TimelineView->searchBar()->show();
    TimelineView->searchBar()->setSearchText(Query->text());
    TimelineView->highlighter()->setHighlight(Query->text());
    TimelineView->highlighter()->selectNext(Query->text());
}

QList<int> SearchTab::sizes() const
{
    QList<int> result = Splitter->sizes();
    result.append(TimelineView->sizes());

    return result;
}

void SearchTab::setSizes(const QList<int> &newSizes)
{
    Q_ASSERT(newSizes.size() == 4);

    Splitter->setSizes(newSizes.mid(0, 2));
    TimelineView->setSizes(newSizes.mid(2, 2));
}
