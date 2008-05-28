#ifndef HISTORY_DIALOG_H
#define HISTORY_DIALOG_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "gadu.h"
#include "history.h"
#include "history_search_dialog.h"

class QTreeWidget;

class ChatMessage;
class ChatMessagesView;

class UinsListViewText : public QTreeWidgetItem
{
		UinsList uins;

	public:
		UinsListViewText(QTreeWidget *parent, const UinsList &uins);
		const UinsList &getUinsList() const;
};

class DateListViewText : public QTreeWidgetItem
{
		HistoryDate date;

	public:
		DateListViewText(QTreeWidgetItem *parent, const HistoryDate &date);
		const HistoryDate &getDate() const;
};

/**
	History dialog
**/
class HistoryDialog : public QWidget
{
	Q_OBJECT

	private slots:
		void showStatusChanged(bool);

	protected:
		ChatMessage * createChatMessage(const HistoryEntry &entry);
		void showHistoryEntries(int from, int count);
		void setDateListViewText(const QDateTime &datetime);
		void searchHistory();
		static const QString &gaduStatus2symbol(unsigned int status);
		void closeEvent(QCloseEvent *e);

		QTreeWidget *uinsTreeWidget;
		ChatMessagesView* body;
		UinsList uins;
		int start;
		HistoryFindRec findRec;
		bool closeDemand;
		bool finding;
		QList<HistoryDate> dateEntries;

		virtual void keyPressEvent(QKeyEvent *e);

	public:
		HistoryDialog(UinsList uins);

	public slots:
		void uinsChanged(QTreeWidgetItem *item);
		void dateChanged(QTreeWidgetItem*item);
		void searchButtonClicked();
		void searchNextButtonClicked();
		void searchPrevButtonClicked();
};

#endif
