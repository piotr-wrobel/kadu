/*
 * %kadu copyright begin%
 * Copyright 2009, 2009, 2009, 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2009, 2010 Piotr Galiszewski (piotrgaliszewski@gmail.com)
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

#ifndef HISTORY_SQL_STORAGE_H
#define HISTORY_SQL_STORAGE_H

#include <QtCore/QMutex>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include "core/crash-aware-object.h"

#include "modules/history/storage/history-storage.h"

/**
	@class HistorySqlStorage
	@author Juzef, Adrian
**/

class HistorySqlStorage : public HistoryStorage, CrashAwareObject
{
	Q_OBJECT

	QSqlDatabase Database;

	QSqlQuery ListChatsQuery;
	QSqlQuery AppendMessageQuery;
	QSqlQuery AppendStatusQuery;
	QSqlQuery AppendSmsQuery;

	QMutex DatabaseMutex;

	void initDatabase();
	void initQueries();
	void initTables();
	void initIndexes();
	void initKaduMessagesTable();
	void initKaduStatusesTable();
	void initKaduSmsTable();

	QString chatWhere(Chat chat);
	QString buddyContactsWhere(Buddy buddy);

	void executeQuery(QSqlQuery query);
	QList<Message> messagesFromQuery(Chat chat, QSqlQuery query);
	QList<Status> statusesFromQuery(QSqlQuery query);

private slots:
	virtual void messageReceived(const Message &message);
	virtual void messageSent(const Message &message);

protected:
	virtual void crash();

public:
	explicit HistorySqlStorage(QObject *parent = 0);
	virtual ~HistorySqlStorage();

	virtual QList<Chat> chats(HistorySearchParameters search);

	virtual QList<QDate> chatDates(Chat chat, HistorySearchParameters search);
	virtual QList<Message> messages(Chat chat, QDate date = QDate(), int limit = 0);
	virtual QList<Message> messagesSince(Chat chat, QDate date);
	virtual QList<Message> messagesBackTo(Chat chat, QDateTime datetime, int limit);

	virtual QList<Buddy> statusBuddiesList(HistorySearchParameters search);
	virtual QList<QDate> datesForStatusBuddy(Buddy buddy, HistorySearchParameters search);
	virtual QList<Status> statuses(Buddy buddy, QDate date = QDate(), int limit = 0);
	virtual int statusBuddyCount(Buddy buddy, QDate date = QDate());

	virtual int messagesCount(Chat chat, QDate date = QDate());
	
	virtual QList<QString> smsReceipientsList(HistorySearchParameters search);
	virtual QList<QDate> datesForSmsReceipient(const QString &receipient, HistorySearchParameters search);
	virtual QList<QString> sms(const QString &receipient, QDate date = QDate(), int limit = 0);
	virtual int smsCount(const QString &receipient, QDate date = QDate());

	virtual void appendMessage(const Message &message);
	virtual void appendStatus(Contact contact, Status status);
	virtual void appendSms(const QString &receipeint, const QString &content);

	void sync();

	void clearChatHistory(Chat chat);
	
	//TODO 0.6.6
	void convertSenderToContact();

};

#endif // HISTORY_SQL_STORAGE_H
