/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qradiobutton.h>
#include <qtextcodec.h>
#include <qtooltip.h>
#include <qvbuttongroup.h>
#include <qwidget.h>

#include <time.h>

#include "config_file.h"
#include "config_dialog.h"
#include "debug.h"
#include "emoticons.h"
#include "history.h"
#include "kadu.h"
#include "misc.h"
#include "status.h"

enum {
	HISTORYMANAGER_ORDINARY_LINE,
	HISTORYMANAGER_HISTORY_OUR,
	HISTORYMANAGER_HISTORY_FOREIGN,
	HISTORYMANAGER_SMS_WITH_NICK,
	HISTORYMANAGER_SMS_WITHOUT_NICK
};

HistoryManager::HistoryManager(QObject *parent, const char *name) : QObject(parent, name)
{
	imagesTimer=new QTimer(this, "imagesTimer");
	imagesTimer->start(1000*60);//60 sekund
	connect(imagesTimer, SIGNAL(timeout()), this, SLOT(checkImagesTimeouts()));
}

QString HistoryManager::text2csv(const QString &text)
{
	QString csv = text;
	csv.replace(QRegExp("\\\\"), "\\\\");
	csv.replace(QRegExp("\""), "\\\"");
	csv.replace(QRegExp("\r\n"), "\\n");
	csv.replace(QRegExp("\n"), "\\n");
	if (csv != text || text.find(QRegExp(","), 0) != -1)
		csv = QString("\"") + csv + QString("\"");
	return csv;
}

QString HistoryManager::getFileNameByUinsList(UinsList &uins)
{
	kdebugf();
	unsigned int i;
	QString fname;
	if (uins.count()) {
		uins.sort();
		for (i = 0; i < uins.count(); ++i)
		{
			fname.append(QString::number(uins[i]));
			if (i < uins.count() - 1)
				fname.append("_");
		}
	}
	else
		fname = "sms";
	kdebugf2();
	return fname;
}

void HistoryManager::appendMessage(UinsList uins, UinType uin, const QString &msg, bool own, time_t czas, bool chat, time_t arriveTime)
{
	kdebugf();
	QFile f, fidx;
	QString fname = ggPath("history/");
	QString line, nick;
	QStringList linelist;
	int offs;

	convHist2ekgForm(uins);
	fname.append(getFileNameByUinsList(uins));

	if (own)
		if (chat)
			linelist.append("chatsend");
		else
			linelist.append("msgsend");
	else
		if (chat)
			linelist.append("chatrcv");
		else
			linelist.append("msgrcv");
	linelist.append(QString::number(uin));
	if (userlist.containsUin(uin))
		nick = userlist.byUin(uin).altNick();
	else
		nick = QString::number(uin);
	linelist.append(text2csv(nick));
	linelist.append(QString::number(arriveTime));
	if (!own)
		linelist.append(QString::number(czas));
	linelist.append(text2csv(msg));
	line = linelist.join(",");

	f.setName(fname);
	if (!(f.open(IO_WriteOnly | IO_Append)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening history file %s\n", (const char *)fname.local8Bit());
		return;
	}

	buildIndexPrivate(fname);
	fidx.setName(f.name() + ".idx");
	if (fidx.open(IO_WriteOnly | IO_Append))
	{
		offs = f.at();
		fidx.writeBlock((const char *)&offs, sizeof(int));
		fidx.close();
	}

	QTextStream stream(&f);
	stream.setCodec(QTextCodec::codecForName("ISO 8859-2"));
	stream << line << '\n';

	f.close();
	kdebugf2();
}

void HistoryManager::appendSms(const QString &mobile, const QString &msg)
{
	kdebugmf(KDEBUG_FUNCTION_START, "appending sms to history (%s)\n", mobile.local8Bit().data());
	QFile f, fidx;
	QTextStream stream;
	QStringList linelist;
	QString altnick, line, fname;
	UinType uin = 0;
	int offs;

	convSms2ekgForm();

	linelist.append("smssend");
	linelist.append(mobile);
	linelist.append(QString::number(time(NULL)));
	linelist.append(text2csv(msg));

	for (UserList::ConstIterator i = userlist.begin(); i != userlist.end(); ++i)
		if ((*i).mobile() == mobile)
		{
			altnick = (*i).altNick();
			uin = (*i).uin();
			break;
		}
	if (uin)
	{
		UinsList uins;
		uins.append(uin);
		convHist2ekgForm(uins);
		linelist.append(text2csv(altnick));
		linelist.append(QString::number(uin));
	}

	line = linelist.join(",");

	f.setName(ggPath("history/sms"));
	if (!(f.open(IO_WriteOnly | IO_Append)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening sms history file\n");
		return;
	}

	buildIndexPrivate(f.name());
	fidx.setName(f.name() + ".idx");
	if (fidx.open(IO_WriteOnly | IO_Append))
	{
		offs = f.at();
		fidx.writeBlock((const char *)&offs, sizeof(int));
		fidx.close();
	}

	stream.setDevice(&f);
	stream.setCodec(QTextCodec::codecForName("ISO 8859-2"));
	stream << line << '\n';
	f.close();

	if (uin)
	{
		fname = ggPath("history/");
		fname = fname + QString::number(uin);
		f.setName(fname);
		if (!(f.open(IO_WriteOnly | IO_Append)))
		{
			kdebugmf(KDEBUG_ERROR, "Error opening sms history\n");
			return;
		}

		fidx.setName(f.name() + ".idx");
		if (fidx.open(IO_WriteOnly | IO_Append))
		{
			offs = f.at();
			fidx.writeBlock((const char *)&offs, sizeof(int));
			fidx.close();
		}

		stream.setDevice(&f);
		stream.setCodec(QTextCodec::codecForName("ISO 8859-2"));
		stream << line << '\n';
		f.close();
	}
	kdebugf2();
}

void HistoryManager::appendStatus(UinType uin, const UserStatus &status)
{
	kdebugf();

	QFile f, fidx;
	QString fname = ggPath("history/");
	QString line, nick, addr;
	QStringList linelist;
	int offs;
	QHostAddress ip;
	unsigned short port;
//	struct in_addr in;

	if (config_file.readBoolEntry("History", "DontSaveStatusChanges"))
	{
		kdebugm(KDEBUG_INFO|KDEBUG_FUNCTION_END, "not appending\n");
		return;
	}

	UinsList uins;
	uins.append(uin);
	convHist2ekgForm(uins);
	linelist.append("status");
	linelist.append(QString::number(uin));
	if (userlist.containsUin(uin))
	{
		nick = userlist.byUin(uin).altNick();
		ip = userlist.byUin(uin).ip();
		port = userlist.byUin(uin).port();
	}
	else
	{
		nick = QString::number(uin);
		ip.setAddress((unsigned int)0);
		port = 0;
	}
	linelist.append(text2csv(nick));
	addr = ip.toString();
	if (port)
		addr = addr + QString(":") + QString::number(port);
	linelist.append(addr);
	linelist.append(QString::number(time(NULL)));
	switch (status.status())
	{
		case Online:
			linelist.append("avail");
			break;
		case Busy:
			linelist.append("busy");
			break;
		case Invisible:
			linelist.append("invisible");
			break;
		case Offline:
		default:
			linelist.append("notavail");
			break;
	}
	if (status.hasDescription())
	{
		QString d = status.description();
		HtmlDocument::escapeText(d);
		linelist.append(text2csv(d));
	}
	line = linelist.join(",");

	fname = fname + QString::number(uin);
	f.setName(fname);
	if (!(f.open(IO_WriteOnly | IO_Append)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening history file %s\n", (const char *)fname.local8Bit());
		return;
	}

	buildIndexPrivate(fname);
	fidx.setName(fname + ".idx");
	if (fidx.open(IO_WriteOnly | IO_Append))
	{
		offs = f.at();
		fidx.writeBlock((const char *)&offs, sizeof(int));
		fidx.close();
	}

	QTextStream stream(&f);
	stream.setCodec(QTextCodec::codecForName("ISO 8859-2"));
	stream << line << '\n';

	f.close();
	kdebugf2();
}

void HistoryManager::removeHistory(UinsList uins)
{
	kdebugf();

	QString fname;
	switch (QMessageBox::information(kadu, "Kadu", qApp->translate("@default",QT_TR_NOOP("Clear history?")),
		qApp->translate("@default",QT_TR_NOOP("Yes")), qApp->translate("@default",QT_TR_NOOP("No")), QString::null, 1, 1))
	{
		case 0:
			fname = ggPath("history/");
			fname.append(getFileNameByUinsList(uins));
			kdebugmf(KDEBUG_INFO, "deleting %s\n", (const char *)fname.local8Bit());
			QFile::remove(fname);
			QFile::remove(fname + ".idx");
			break;
		case 1:
			break;
	}
	kdebugf2();
}

void HistoryManager::convHist2ekgForm(UinsList uins)
{
	kdebugf();

	QFile f, fout;
	QString path = ggPath("history/");
	QString fname, fnameout, line, nick;
	QStringList linelist;
	UinType uin;

	fname = getFileNameByUinsList(uins);

	f.setName(path + fname);
	if (!(f.open(IO_ReadWrite)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening history file %s\n", (const char *)fname.local8Bit());
		return;
	}

	fnameout = fname + ".new";
	fout.setName(path + fnameout);
	if (!(fout.open(IO_WriteOnly | IO_Truncate)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening new history file %s\n", (const char *)fnameout.local8Bit());
		f.close();
		return;
	}

	QTextStream stream(&f);
	stream.setCodec(QTextCodec::codecForName("ISO 8859-2"));
	QTextStream streamout(&fout);
	streamout.setCodec(QTextCodec::codecForName("ISO 8859-2"));

	bool our, foreign;
	QString dzien, miesiac, rok, czas, sczas, text, temp, lineout;
	QDateTime datetime, sdatetime;
	QRegExp sep("\\s"), sep2("::");
	our = foreign = false;
	UinType myUin=config_file.readNumEntry("General","UIN");
	while ((line = stream.readLine()) != QString::null)
	{
//		our = !line.find(QRegExp("^\\S+\\s::\\s\\d{2,2}\\s\\d{2,2}\\s\\d{4,4},\\s\\(\\d{2,2}:\\d{2,2}:\\d{2,2}\\)$"));
		our = !line.find(QRegExp("^(\\S|\\s)+\\s::\\s\\d{2,2}\\s\\d{2,2}\\s\\d{4,4},\\s\\(\\d{2,2}:\\d{2,2}:\\d{2,2}\\)$"));
//		foreign = !line.find(QRegExp("^\\S+\\s::\\s\\d{2,2}\\s\\d{2,2}\\s\\d{4,4},\\s\\(\\d{2,2}:\\d{2,2}:\\d{2,2}\\s/\\sS\\s\\d{2,2}:\\d{2,2}:\\d{2,2}\\)$"));
		foreign = !line.find(QRegExp("^(\\S|\\s)+\\s::\\s\\d{2,2}\\s\\d{2,2}\\s\\d{4,4},\\s\\(\\d{2,2}:\\d{2,2}:\\d{2,2}\\s/\\sS\\s\\d{2,2}:\\d{2,2}:\\d{2,2}\\)$"));
		if (our || foreign)
		{
			if (linelist.count())
			{
				text.truncate(text.length() - 1);
				if (text[text.length() - 1] == '\n')
					text.truncate(text.length() - 1);
				linelist.append(text2csv(text));
				lineout = linelist.join(",");
				streamout << lineout << '\n';
			}
			linelist.clear();
			text.truncate(0);
			nick = line.section(sep2, 0, 0);
			nick.truncate(nick.length() - 1);
			line = line.right(line.length() - nick.length() - 4);
			dzien = line.section(sep, 0, 0);
			miesiac = line.section(sep, 1, 1);
			rok = line.section(sep, 2, 2);
			rok.truncate(rok.length() - 1);
			datetime.setDate(QDate(rok.toInt(), miesiac.toInt(), dzien.toInt()));
			sdatetime = datetime;
			czas = line.section(sep, 3, 3);
			czas.remove(0, 1);
			if (our)
			{
				czas.truncate(czas.length() - 1);
				linelist.append("chatsend");
			}
			datetime.setTime(QTime(czas.left(2).toInt(), czas.mid(3, 2).toInt(), czas.right(2).toInt()));
			if (foreign)
			{
				sczas = line.section(sep, 6, 6);
				sczas.truncate(sczas.length() - 1);
				sdatetime.setTime(QTime(sczas.left(2).toInt(), sczas.mid(3, 2).toInt(), sczas.right(2).toInt()));
				linelist.append("chatrcv");
			}
			if (our)
			{
				if (uins.count() > 1)
					uin = 0;
				else if (myUin != uins[0])
					uin = uins[0];
				else
					uin = uins[1];
			}
			else if (userlist.containsAltNick(nick))
			{
				UserListElement &user = userlist.byAltNick(nick);
				uin = user.uin();
			}
			else if (uins.count() > 1)
				uin = 0;
			else if (myUin != uins[0])
				uin = uins[0];
			else
				uin = uins[1];
			linelist.append(QString::number(uin));
			if (our)
				if (userlist.containsUin(uin))
					nick = userlist.byUin(uin).altNick();
				else
					nick = QString::number(uin);
			linelist.append(nick);
			linelist.append(QString::number(-datetime.secsTo(
				QDateTime(QDate(1970, 1, 1), QTime(0 ,0)))));
			if (foreign)
				linelist.append(QString::number(-sdatetime.secsTo(
					QDateTime(QDate(1970, 1, 1), QTime(0 ,0)))));
			our = foreign = false;
		}
		else
		{
			if (!linelist.count())
				break;
			text.append(line);
			text.append("\n");
		}
	}
	if (linelist.count())
	{
		text.truncate(text.length() - 1);
		if (text[text.length() - 1] == '\n')
			text.truncate(text.length() - 1);
		linelist.append(text2csv(text));
		lineout = linelist.join(",");
		streamout << lineout << '\n';
		f.close();
		fout.close();
		QDir dir(path);
		dir.rename(fname, fname + QString(".old"));
		dir.rename(fnameout, fname);
	}
	else
	{
		f.close();
		fout.remove();
	}
	kdebugf2();
}

void HistoryManager::convSms2ekgForm()
{
	kdebugf();

	QFile f, fout;
	QString path = ggPath("history/");
	QString fname, fnameout, line, nick;
	QStringList linelist;
	UinType uin=0;

	fname = "sms";
	f.setName(path + fname);
	if (!(f.open(IO_ReadWrite)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening sms history file %s\n", (const char *)fname.local8Bit());
		return;
	}
	fnameout = fname + ".new";
	fout.setName(path + fnameout);
	if (!(fout.open(IO_WriteOnly | IO_Truncate)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening new sms history file %s\n", (const char *)fnameout.local8Bit());
		f.close();
		return;
	}

	QTextStream stream(&f);
	stream.setCodec(QTextCodec::codecForName("ISO 8859-2"));
	QTextStream streamout(&fout);
	streamout.setCodec(QTextCodec::codecForName("ISO 8859-2"));

	bool header;
	QString mobile, dzien, miesiac, rok, czas, text, temp, lineout;
	QDateTime datetime;
	QRegExp sep("\\s");
	header = false;
	while ((line = stream.readLine()) != QString::null)
	{
		header = !line.find(QRegExp("^\\S+\\s\\(\\d+\\)\\s::\\s\\d{2,2}\\s\\d{2,2}\\s\\d{4,4},\\s\\(\\d{2,2}:\\d{2,2}:\\d{2,2}\\)$"));
		if (header)
		{
			if (linelist.count())
			{
				text.truncate(text.length() - 1);
				if (text[text.length() - 1] == '\n')
					text.truncate(text.length() - 1);
				linelist.append(text2csv(text));
				if (uin)
				{
					linelist.append(nick);
					linelist.append(QString::number(uin));
				}
				lineout = linelist.join(",");
				streamout << lineout << '\n';
			}
			linelist.clear();
			text.truncate(0);
			nick = line.section(sep, 0, 0);
			uin = 0;
			mobile = line.section(sep, 1, 1);
			mobile.remove(0, 1);
			mobile.truncate(mobile.length() - 1);
			dzien = line.section(sep, 3, 3);
			miesiac = line.section(sep, 4, 4);
			rok = line.section(sep, 5, 5);
			rok.truncate(rok.length() - 1);
			datetime.setDate(QDate(rok.toInt(), miesiac.toInt(), dzien.toInt()));
			czas = line.section(sep, 6, 6);
			czas.remove(0, 1);
			czas.truncate(czas.length() - 1);
			linelist.append("smssend");
			linelist.append(mobile);
			datetime.setTime(QTime(czas.left(2).toInt(), czas.mid(3, 2).toInt(), czas.right(2).toInt()));
			linelist.append(QString::number(-datetime.secsTo(
				QDateTime(QDate(1970, 1, 1), QTime(0 ,0)))));
			for (UserList::ConstIterator i = userlist.begin(); i != userlist.end(); ++i)
				if ((*i).mobile() == mobile)
					uin = (*i).uin();
			header = false;
		}
		else
		{
			if (!linelist.count())
				break;
			text.append(line);
			text.append("\n");
		}
	}
	if (linelist.count())
	{
		text.truncate(text.length() - 1);
		if (text[text.length() - 1] == '\n')
			text.truncate(text.length() - 1);
		linelist.append(text2csv(text));
		if (uin)
		{
			linelist.append(nick);
			linelist.append(QString::number(uin));
		}
		lineout = linelist.join(",");
		streamout << lineout << '\n';
		f.close();
		fout.close();
		QDir dir(path);
		dir.rename(fname, fname + QString(".old"));
		dir.rename(fnameout, fname);
	}
	else
	{
		f.close();
		fout.remove();
	}
	kdebugf2();
}

int HistoryManager::getHistoryEntriesCountPrivate(const QString &filename)
{
	kdebugf();

	int lines;
	QFile f;
	QString path = ggPath("history/");
	QByteArray buffer;

	f.setName(path + filename + ".idx");
	if (!(f.open(IO_ReadOnly)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening history file %s\n", (const char *)filename.local8Bit());
		return 0;
	}
	lines = f.size() / sizeof(int);
//	buffer = f.readAll();
	f.close();
//	lines = buffer.contains('\n');

	kdebugmf(KDEBUG_INFO, "%d lines\n", lines);
	return lines;
}

int HistoryManager::getHistoryEntriesCount(UinsList uins)
{
	kdebugf();
	convHist2ekgForm(uins);
	buildIndex(uins);
	int ret=getHistoryEntriesCountPrivate(getFileNameByUinsList(uins));
	kdebugf2();
	return ret;
}

int HistoryManager::getHistoryEntriesCount(QString mobile)
{
	kdebugf();
	convSms2ekgForm();
	buildIndex();
	int ret;
	if (mobile == QString::null)
		ret= getHistoryEntriesCountPrivate("sms");
	else
		ret= getHistoryEntriesCountPrivate(mobile);
	kdebugf2();
	return ret;
}

QValueList<HistoryEntry> HistoryManager::getHistoryEntries(UinsList uins, int from, int count, int mask)
{
	kdebugf();

	QValueList<HistoryEntry> entries;
	QStringList tokens;
	QFile f, fidx;
	QString path = ggPath("history/");
	QString filename, line;
	int offs;

	if (uins.count())
		filename = getFileNameByUinsList(uins);
	else
		filename = "sms";
	f.setName(path + filename);
	if (!(f.open(IO_ReadOnly)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening history file %s\n", (const char *)filename.local8Bit());
		return entries;
	}

	fidx.setName(f.name() + ".idx");
	if (!fidx.open(IO_ReadOnly))
		return entries;
	fidx.at(from * sizeof(int));
	fidx.readBlock((char *)&offs, (Q_LONG)sizeof(int));
	fidx.close();
	if (!f.at(offs))
		return entries;

	QTextStream stream(&f);
	stream.setCodec(QTextCodec::codecForName("ISO 8859-2"));

	int linenr = from;

	struct HistoryEntry entry;
	while (linenr < from + count && (line = stream.readLine()) != QString::null)
	{
		++linenr;
		tokens = mySplit(',', line);
		if (tokens.count() < 2)
			continue;
		if (tokens[0] == "chatsend")
			entry.type = HISTORYMANAGER_ENTRY_CHATSEND;
		else if (tokens[0] == "msgsend")
			entry.type = HISTORYMANAGER_ENTRY_MSGSEND;
		else if (tokens[0] == "chatrcv")
			entry.type = HISTORYMANAGER_ENTRY_CHATRCV;
		else if (tokens[0] == "msgrcv")
			entry.type = HISTORYMANAGER_ENTRY_MSGRCV;
		else if (tokens[0] == "status")
			entry.type = HISTORYMANAGER_ENTRY_STATUS;
		else if (tokens[0] == "smssend")
			entry.type = HISTORYMANAGER_ENTRY_SMSSEND;
		if (!(entry.type & mask))
			continue;
		switch (entry.type)
		{
			case HISTORYMANAGER_ENTRY_CHATSEND:
			case HISTORYMANAGER_ENTRY_MSGSEND:
				if (tokens.count() == 5)
				{
					entry.uin = tokens[1].toUInt();
					entry.nick = tokens[2];
					entry.date.setTime_t(tokens[3].toUInt());
					entry.message = tokens[4];
					entry.ip.truncate(0);
					entry.mobile.truncate(0);
					entry.description.truncate(0);
					entries.append(entry);
				}
				break;
			case HISTORYMANAGER_ENTRY_CHATRCV:
			case HISTORYMANAGER_ENTRY_MSGRCV:
				if (tokens.count() == 6)
				{
					entry.uin = tokens[1].toUInt();
					entry.nick = tokens[2];
					entry.date.setTime_t(tokens[3].toUInt());
					entry.sdate.setTime_t(tokens[4].toUInt());
					entry.message = tokens[5];
					entry.ip.truncate(0);
					entry.mobile.truncate(0);
					entry.description.truncate(0);
					entries.append(entry);
				}
				break;
			case HISTORYMANAGER_ENTRY_STATUS:
				if (tokens.count() == 6 || tokens.count() == 7)
				{
					entry.uin = tokens[1].toUInt();
					entry.nick = tokens[2];
					entry.ip = tokens[3];
					entry.date.setTime_t(tokens[4].toUInt());
					if (tokens[5] == "avail")
						entry.status = GG_STATUS_AVAIL;
					else if (tokens[5] == "notavail")
						entry.status = GG_STATUS_NOT_AVAIL;
					else if (tokens[5] == "busy")
						entry.status = GG_STATUS_BUSY;
					else if (tokens[5] == "invisible")
						entry.status = GG_STATUS_INVISIBLE;
					if (tokens.count() == 7)
					{
						switch (entry.status)
						{
							case GG_STATUS_AVAIL:
								entry.status = GG_STATUS_AVAIL_DESCR;
								break;
							case GG_STATUS_NOT_AVAIL:
								entry.status = GG_STATUS_NOT_AVAIL_DESCR;
								break;
							case GG_STATUS_BUSY:
								entry.status = GG_STATUS_BUSY_DESCR;
								break;
							case GG_STATUS_INVISIBLE:
								entry.status = GG_STATUS_INVISIBLE_DESCR;
								break;
						}
						entry.description = tokens[6];
					}
					else
						entry.description.truncate(0);
					entry.mobile.truncate(0);
					entry.message.truncate(0);
					entries.append(entry);
				}
				break;
			case HISTORYMANAGER_ENTRY_SMSSEND:
				if (tokens.count() == 4 || tokens.count() == 6)
				{
					entry.mobile = tokens[1];
					entry.date.setTime_t(tokens[2].toUInt());
					entry.message = tokens[3];
					if (tokens.count() == 4)
					{
						entry.nick.truncate(0);
						entry.uin = 0;
					}
					else
					{
						entry.nick = tokens[4];
						entry.uin = tokens[5].toUInt();
					}
					entry.ip.truncate(0);
					entry.description.truncate(0);
					entries.append(entry);
				}
				break;
		}
	}

	f.close();

	kdebugf2();
	return entries;
}

uint HistoryManager::getHistoryDate(QTextStream &stream)
{
	kdebugf();
	QString line;
	static QStringList types = QStringList::split(" ", "smssend chatrcv chatsend msgrcv msgsend status");
	QStringList tokens;
	int type, pos;

	line = stream.readLine();
	tokens = mySplit(',', line);
	type = types.findIndex(tokens[0]);
	if (!type)
		pos = 2;
	else
		if (type < 5)
			pos = 3;
		else
			pos = 4;
	kdebugf2();
	return (tokens[pos].toUInt() / 86400);
}

QValueList<HistoryDate> HistoryManager::getHistoryDates(UinsList uins)
{
	kdebugf();

	QValueList<HistoryDate> entries;
	HistoryDate newdate;
	QFile f, fidx;
	QString path = ggPath("history/");
	QString filename, line;
	uint offs, count, oldidx, actidx, leftidx, rightidx, /*mididx,*/ olddate, actdate, jmp;

	if (uins.count())
		count = getHistoryEntriesCount(uins);
	else
		count = getHistoryEntriesCount("sms");
	if (!count)
		return entries;

	filename = getFileNameByUinsList(uins);
	f.setName(path + filename);
	if (!(f.open(IO_ReadOnly)))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening history file %s\n", (const char *)filename.local8Bit());
		return entries;
	}
	QTextStream stream(&f);
	stream.setCodec(QTextCodec::codecForName("ISO 8859-2"));

	fidx.setName(f.name() + ".idx");
	if (!fidx.open(IO_ReadOnly))
		return entries;

	oldidx = actidx = 0;
	olddate = actdate = getHistoryDate(stream);
	kdebugmf(KDEBUG_INFO, "actdate = %d\n", actdate);
	newdate.idx = 0;
	newdate.date.setTime_t(actdate * 86400);
	entries.append(newdate);

	while (actidx < count - 1)
	{
		jmp = 1;
		do
		{
			oldidx = actidx;
			actidx += jmp;
			jmp <<= 1;
			if (jmp > 128)
				jmp = 128;
			if (actidx >= count)
				actidx = count - 1;
			if (actidx == oldidx)
				break;
			fidx.at(actidx * sizeof(int));
			fidx.readBlock((char *)&offs, (Q_LONG)sizeof(int));
			f.at(offs);
			actdate = getHistoryDate(stream);
		} while (actdate == olddate);

		if (actidx == oldidx)
			break;
		if (actdate > olddate)
		{
			leftidx = oldidx;
			rightidx = actidx;
			while (rightidx - leftidx > 1)
			{
				actidx = (leftidx + rightidx) / 2;
				fidx.at(actidx * sizeof(int));
				fidx.readBlock((char *)&offs, (Q_LONG)sizeof(int));
				f.at(offs);
				actdate = getHistoryDate(stream);
				if (actdate > olddate)
					rightidx = actidx;
				else
					leftidx = actidx;
			}
			newdate.idx = actidx = rightidx;
			if (actdate == olddate)
			{
				fidx.at(actidx * sizeof(int));
				fidx.readBlock((char *)&offs, (Q_LONG)sizeof(int));
				f.at(offs);
				actdate = getHistoryDate(stream);
			}
			newdate.date.setTime_t(actdate * 86400);
			entries.append(newdate);
			olddate = actdate;
		}
	}

	fidx.close();
	f.close();

	kdebugf2();
	return entries;
}

QValueList<UinsList> HistoryManager::getUinsLists()
{
	kdebugf();
	QValueList<UinsList> entries;
	QDir dir(ggPath("history/"), "*.idx");
	QStringList struins;
	UinsList uins;

	for (unsigned int i = 0; i < dir.count(); ++i)
	{
		struins = QStringList::split("_", dir[i].replace(QRegExp(".idx$"), ""));
		uins.clear();
		if (struins[0] != "sms")
			for (unsigned int j = 0; j < struins.count(); ++j)
				uins.append(struins[j].toUInt());
		entries.append(uins);
	}

	kdebugf2();
	return entries;
}

void HistoryManager::buildIndexPrivate(const QString &filename)
{
	kdebugf();
	QString fnameout = filename + ".idx";
	char *inbuf;
	int *outbuf;
	int inbufoffs, outbufoffs, inoffs;
	Q_LONG read, written;
	bool saved = false;

	if (QFile::exists(fnameout))
		return;
	QFile fin(filename);
	QFile fout(fnameout);
	if (!fin.open(IO_ReadOnly))
	{
		kdebugmf(KDEBUG_ERROR, "Error opening history file: %s\n", (const char *)fin.name().local8Bit());
		return;
	}
	if (!fout.open(IO_WriteOnly | IO_Truncate))
	{
		kdebugmf(KDEBUG_ERROR, "Error creating history index file: %s\n", (const char *)fout.name().local8Bit());
		fin.close();
		return;
	}
	inbuf = new char[65536];
	outbuf = new int[4096];

	inoffs = outbufoffs = 0;
	while ((read = fin.readBlock(inbuf, 65536)) > 0)
	{
		inbufoffs = 0;
		while (inbufoffs < read)
		{
			if (saved)
				saved = false;
			else
				outbuf[outbufoffs++] = inoffs + inbufoffs;
			if (outbufoffs == 4096)
			{
				written = fout.writeBlock((char *)outbuf, 4096 * sizeof(int));
				outbufoffs = 0;
			}
			while (inbufoffs < read && inbuf[inbufoffs] != '\n')
				++inbufoffs;
			if (inbufoffs < read)
				++inbufoffs;
			if (inbufoffs == read)
			{
				inoffs += read;
				saved = true;
			}
		}
	}
	if (outbufoffs)
		written = fout.writeBlock((char *)outbuf, outbufoffs * sizeof(int));

	delete []inbuf;
	delete []outbuf;

	fin.close();
	fout.close();
	kdebugf2();
}

void HistoryManager::buildIndex(UinsList uins)
{
	kdebugf();
	buildIndexPrivate(ggPath("history/") + getFileNameByUinsList(uins));
	kdebugf2();
}

void HistoryManager::buildIndex(QString mobile)
{
	kdebugf();
	if (mobile == QString::null)
		buildIndexPrivate(ggPath("history/") + "sms");
	else
		buildIndexPrivate(ggPath("history/") + mobile);
	kdebugf2();
}


QStringList HistoryManager::mySplit(const QChar &sep, const QString &str)
{
	kdebugf();
	QStringList strlist;

	QString token;
	QChar letter;
	unsigned int idx = 0, state = 0;
	while (idx < str.length())
	{
		letter = str[idx];
		switch (state)
		{
			case 0:
				if (letter == sep)
				{
					if (token.length())
						token.truncate(0);
					else
						strlist.append(token);
				}
				else if (letter == '"')
					state = 2;
				else
				{
					token.append(letter);
					state = 1;
				}
				++idx;
				break;
			case 1:
				if (letter != sep)
				{
					token.append(letter);
					++idx;
				}
				else
				{
					strlist.append(token);
					state = 0;
				}
				break;
			case 2:
				if (letter == '\\')
					state = 3;
				else
					if (letter == '\"')
					{
						strlist.append(token);
						state = 0;
					}
					else
						token.append(letter);
				++idx;
				break;
			case 3:
				switch (letter)
				{
					case 'n':
						token.append('\n');
						break;
					case '\\':
						token.append('\\');
						break;
					case '\"':
						token.append('\"');
						break;
					default:
						token.append('?');
				}
				state = 2;
				++idx;
				break;
		}
	}
	if (state == 1)
		strlist.append(token);

	kdebugf2();
	return strlist;
}

int HistoryManager::getHistoryEntryIndexByDate(UinsList uins, QDateTime &date, bool enddate)
{
	kdebugf();

	QValueList<HistoryEntry> entries;
	int count = getHistoryEntriesCount(uins);
	int start, end;

	start = 0;
	end = count - 1;
	while (end - start >= 0)
	{
		kdebugmf(KDEBUG_INFO, "start = %d, end = %d\n", start, end);
		entries = getHistoryEntries(uins, start + ((end - start) / 2), 1);
		if (entries.count())
			if (date < entries[0].date)
				end -= ((end - start) / 2) + 1;
			else
				if (date > entries[0].date)
					start += ((end - start) / 2) + 1;
				else
					return start + ((end - start) / 2);
	}
	if (end < 0)
	{
		kdebugmf(KDEBUG_FUNCTION_END, "return 0\n");
		return 0;
	}
	if (start >= count)
	{
		kdebugmf(KDEBUG_FUNCTION_END, "return count=%d\n", count);
		return count;
	}
	if (enddate)
	{
		entries = getHistoryEntries(uins, start, 1);
		if (entries.count() && date < entries[0].date)
			--start;
	}
	kdebugmf(KDEBUG_FUNCTION_END, "return %d\n", start);
	return start;
}

void HistoryManager::chatMsgReceived(UinsList senders, const QString& msg, time_t t, bool& /*grab*/)
{
	if (!config_file.readBoolEntry("History", "Logging"))
		return;
	kdebugf();
	int occur=msg.contains(QRegExp("<img [^>]* gg_crc[^>]*>"));
	kdebugm(KDEBUG_INFO, "sender: %d msg: '%s' occur:%d\n", senders[0], msg.local8Bit().data(), occur);
	if (bufferedMessages.find(senders[0])!=bufferedMessages.end() || occur>0)
	{
		kdebugm(KDEBUG_INFO, "buffering\n");
		bufferedMessages[senders[0]].append(BuffMessage(senders, msg, t, time(NULL), false, occur));
		checkImageTimeout(senders[0]);
	}
	else
	{
		kdebugm(KDEBUG_INFO, "appending to history\n");
		history.appendMessage(senders, senders[0], msg, false, t, true, time(NULL));
	}
	kdebugf2();
}

void HistoryManager::imageReceivedAndSaved(UinType sender, uint32_t size, uint32_t crc32, const QString &path)
{
	if (!config_file.readBoolEntry("History", "Logging"))
		return;
	kdebugf();
	kdebugm(KDEBUG_INFO, "sender: %d, size: %d, crc:%u, path:%s\n", sender, size, crc32, path.local8Bit().data());
	QRegExp reg=QRegExp(GaduImagesManager::loadingImageHtml(sender, size, crc32));
	QString imagehtml=GaduImagesManager::imageHtml(path);
	QMap<UinType, QValueList<BuffMessage> >::iterator it=bufferedMessages.find(sender);
	if (it!=bufferedMessages.end())
	{
//		kdebugm(KDEBUG_INFO, "sender found\n");
		QValueList<BuffMessage> &messages=it.data();
		QValueList<BuffMessage>::iterator msgsEnd=messages.end();
		for (QValueList<BuffMessage>::iterator msg=messages.begin(); msg!=msgsEnd; msg++)
		{
//			kdebugm(KDEBUG_INFO, "counter:%d\n", (*msg).counter);
			if ((*msg).counter)
			{
				int occur=(*msg).message.contains(reg);
//				kdebugm(KDEBUG_INFO, "occur:%d\n", occur);
				if (occur)
				{
					(*msg).message.replace(reg, imagehtml);
					(*msg).counter-=occur;
				}
			}
		}
//		kdebugm(KDEBUG_INFO, "> msgs.size():%d\n", messages.size());
		while (messages.size()>0)
		{
			BuffMessage &msg=messages.front();
			if (msg.counter>0)
				break;
			history.appendMessage(msg.uins, msg.uins[0], msg.message, msg.own, msg.tm, true, msg.arriveTime);
			messages.pop_front();
		}
//		kdebugm(KDEBUG_INFO, ">> msgs.size():%d\n", messages.size());
		if (messages.size()==0)
			bufferedMessages.remove(sender);
	}
	kdebugf2();
}

void HistoryManager::addMyMessage(const UinsList &senders, const QString &msg)
{
	if (!config_file.readBoolEntry("History", "Logging"))
		return;
	kdebugf();
	time_t current=time(NULL);
	if (bufferedMessages.find(senders[0])!=bufferedMessages.end())
	{
		bufferedMessages[senders[0]].append(BuffMessage(senders, msg, 0, current, true, 0));
		checkImageTimeout(senders[0]);
	}
	else
		history.appendMessage(senders, senders[0], msg, true, 0, true, current);
	kdebugf2();
}

void HistoryManager::checkImageTimeout(UinType uin)
{
	kdebugf();
	time_t currentTime=time(NULL);
	QValueList<BuffMessage> &msgs=bufferedMessages[uin];
	while (msgs.count()>0)
	{
		BuffMessage &msg=msgs.front();
		kdebugm(KDEBUG_INFO, "arriveTime:%d current:%d counter:%d\n", msg.arriveTime, currentTime, msg.counter);
		if (msg.arriveTime+60<currentTime || msg.counter==0)
		{
			kdebugm(KDEBUG_INFO, "moving message to history\n");
			history.appendMessage(msg.uins, msg.uins[0], msg.message, msg.own, msg.tm, true, msg.arriveTime);
			msgs.pop_front();
		}
		else
		{
			kdebugm(KDEBUG_INFO, "it's too early\n");
			break;
		}
	}
	if (msgs.count()==0)
		bufferedMessages.remove(uin);
	kdebugf2();
}

void HistoryManager::checkImagesTimeouts()
{
	kdebugf();
	QValueList<UinType> uins=keys(bufferedMessages);
	
	for (QValueList<UinType>::iterator it=uins.begin();
		it!=uins.end();
		it++)
		checkImageTimeout(*it);
	kdebugf2();
}

UinsListViewText::UinsListViewText(QListView *parent, UinsList &uins)
	: QListViewItem(parent), uins(uins)
{
	kdebugf();
	QString name;

	if (!uins.count())
		setText(0, "SMS");
	else
	{
		for (unsigned int i = 0; i < uins.count(); ++i)
		{
			if (userlist.containsUin(uins[i]))
				name.append(userlist.byUin(uins[i]).altNick());
			else
				name.append(QString::number(uins[i]));
			if (i < uins.count() - 1)
				name.append(",");
		}
		setText(0, name);
	}
	kdebugf2();
}

UinsList &UinsListViewText::getUinsList()
{
	return uins;
}

DateListViewText::DateListViewText(QListViewItem *parent, HistoryDate &date)
	: QListViewItem(parent), date(date)
{
	setText(0, date.date.toString("yyyy.MM.dd"));
}

HistoryDate &DateListViewText::getDate()
{
	return date;
}

History::History(UinsList uins) : QDialog(NULL, "HistoryDialog"), uins(uins), closeDemand(false), finding(false)
{
	kdebugf();
	history.convHist2ekgForm(uins);
	history.buildIndex(uins);

	setCaption(tr("History"));
	setWFlags(Qt::WDestructiveClose);

	QGridLayout *grid = new QGridLayout(this, 2, 5, 3, 3, "grid");

	QSplitter *splitter = new QSplitter(Qt::Horizontal, this, "splitter");

	uinslv = new QListView(splitter, "uinslv");
	uinslv->addColumn(tr("Uins"));
	uinslv->setRootIsDecorated(TRUE);

	QVBox *vbox = new QVBox(splitter, "vbox");
	body = new KaduTextBrowser(vbox, "body");
	body->setReadOnly(true);
	body->QTextEdit::setFont(config_file.readFontEntry("Look", "ChatFont"));
	if((EmoticonsStyle)config_file.readNumEntry("Chat","EmoticonsStyle")==EMOTS_ANIMATED)
		body->setStyleSheet(new AnimStyleSheet(body, emoticons->themePath()));
	else
		body->setStyleSheet(new StaticStyleSheet(body,emoticons->themePath()));
	ParagraphSeparator=config_file.readNumEntry("General", "ParagraphSeparator");
	body->setMargin(ParagraphSeparator);

	QHBox *btnbox = new QHBox(vbox, "btnbox");
	QPushButton *searchbtn = new QPushButton(tr("&Find"), btnbox, "searchbtn");
	QPushButton *searchnextbtn = new QPushButton(tr("Find &next"), btnbox, "searcgnextbtn");
	QPushButton *searchprevbtn = new QPushButton(tr("Find &previous"), btnbox, "searchprevbtn");

	QValueList<int> sizes;
	sizes.append(1);
	sizes.append(3);
	splitter->setSizes(sizes);
	grid->addMultiCellWidget(splitter, 0, 1, 0, 4);

	connect(uinslv, SIGNAL(expanded(QListViewItem *)), this, SLOT(uinsChanged(QListViewItem *)));
	connect(uinslv, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(dateChanged(QListViewItem *)));
	connect(searchbtn, SIGNAL(clicked()), this, SLOT(searchBtnClicked()));
	connect(searchnextbtn, SIGNAL(clicked()), this, SLOT(searchNextBtnClicked()));
	connect(searchprevbtn, SIGNAL(clicked()), this, SLOT(searchPrevBtnClicked()));

	loadGeometry(this, "History", "HistoryGeometry", 0, 0, 500, 400);

	findrec.type = 1;
	findrec.reverse = 0;
	findrec.actualrecord = -1;

	UinsListViewText *uinslvt, *selecteduinslvt = NULL;
	QListViewItem *datelvt;

	QValueList<UinsList> uinsentries = history.getUinsLists();
	for (unsigned int i = 0; i < uinsentries.count(); ++i)
	{
		uinslvt = new UinsListViewText(uinslv, uinsentries[i]);
		uinslvt->setExpandable(TRUE);
		if (uinsentries[i].equals(uins) && uins.count())
			selecteduinslvt = uinslvt;
	}
	uinslv->sort();
	if (selecteduinslvt)
	{
		selecteduinslvt->setOpen(TRUE);
		datelvt = selecteduinslvt->firstChild();
		if (datelvt)
		{
			while (datelvt->nextSibling())
				datelvt = datelvt->nextSibling();
			uinslv->setCurrentItem(datelvt);
			uinslv->setSelected(datelvt, TRUE);
			uinslv->ensureItemVisible(datelvt);
		}
	}
	kdebugf2();
}

void History::uinsChanged(QListViewItem *item)
{
	kdebugf();
	QValueList<HistoryDate> dateentries;
	if (item->depth() == 0)
	{
		uins = ((UinsListViewText *)item)->getUinsList();
		if (!item->childCount())
		{
			dateentries = history.getHistoryDates(uins);
			for (unsigned int i = 0; i < dateentries.count(); ++i)
				(new DateListViewText(item, dateentries[i]))->setExpandable(FALSE);
		}
	}
	kdebugf2();
}

void History::dateChanged(QListViewItem *item)
{
	kdebugf();
	int count, depth = item->depth();
	switch (depth)
	{
		case 1:
			uinsChanged(item->parent());
			start = ((DateListViewText *)item)->getDate().idx;
			item = item->nextSibling();
			break;
		case 0:
			uinsChanged(item);
			start = 0;
			item = item->firstChild();
			if (item)
				item = item->nextSibling();
			break;
	}
	count = history.getHistoryEntriesCount(uins);
	if (depth < 2)
	{
		if (item)
			count = ((DateListViewText *)item)->getDate().idx - start;
		else
			count -= start;
		showHistoryEntries(start, count);
	}
	kdebugf2();
}

void History::formatHistoryEntry(QString &text, const HistoryEntry &entry, QStringList &paracolors)
{
	kdebugf();
	QString bgcolor, textcolor;
	QString message;

	message = entry.message;
	message.replace(QRegExp("\n"), "<br/>");

	if (entry.type & (HISTORYMANAGER_ENTRY_CHATSEND | HISTORYMANAGER_ENTRY_MSGSEND
		| HISTORYMANAGER_ENTRY_SMSSEND))
	{
		bgcolor = config_file.readColorEntry("Look","ChatMyBgColor").name();
		textcolor = config_file.readColorEntry("Look","ChatMyFontColor").name();
	}
	else
	{
		bgcolor = config_file.readColorEntry("Look","ChatUsrBgColor").name();
		textcolor = config_file.readColorEntry("Look","ChatUsrFontColor").name();
	}

	const static QString format("<p style=\"background-color: %1\"><img title=\"\" height=\"%3\" width=\"10000\" align=\"right\"><font color=\"%2\"><b>");

	text.append(format.arg(bgcolor).arg(textcolor).arg(ParagraphSeparator));
	paracolors.append(bgcolor);

	if (entry.type == HISTORYMANAGER_ENTRY_SMSSEND)
		text.append(entry.mobile + " SMS");
	else
	{
		QString nick;
		if (entry.type & (HISTORYMANAGER_ENTRY_CHATSEND | HISTORYMANAGER_ENTRY_MSGSEND))
			nick = config_file.readEntry("General","Nick");
		else
			nick = entry.nick;
		HtmlDocument::escapeText(nick);
		text.append(nick);
	}

	text.append(QString(" :: ") + printDateTime(entry.date));
	if (entry.type & (HISTORYMANAGER_ENTRY_CHATRCV | HISTORYMANAGER_ENTRY_MSGRCV))
		text.append(QString(" / S ") + printDateTime(entry.sdate));
	text.append("</b><br/>");
	if (entry.type & HISTORYMANAGER_ENTRY_STATUS)
	{
		switch (entry.status)
		{
			case GG_STATUS_AVAIL:
			case GG_STATUS_AVAIL_DESCR:
				text.append(tr("Online"));
				break;
			case GG_STATUS_BUSY:
			case GG_STATUS_BUSY_DESCR:
				text.append(tr("Busy"));
				break;
			case GG_STATUS_INVISIBLE:
			case GG_STATUS_INVISIBLE_DESCR:
				text.append(tr("Invisible"));
				break;
			case GG_STATUS_NOT_AVAIL:
			case GG_STATUS_NOT_AVAIL_DESCR:
				text.append(tr("Offline"));
				break;
		}
		if (entry.description.length())
			text.append(QString(" (") + entry.description + ")");
		text.append(QString(" ip=") + entry.ip);
	}
	else
	{
		HtmlDocument doc;
		doc.parseHtml(message);
		doc.convertUrlsToHtml();

	 	if((EmoticonsStyle)config_file.readNumEntry("Chat","EmoticonsStyle")!=EMOTS_NONE && config_file.readBoolEntry("General", "ShowEmotHist"))
	 	{
			body->mimeSourceFactory()->addFilePath(emoticons->themePath());
			emoticons->expandEmoticons(doc, bgcolor);
		}
		GaduImagesManager::setBackgroundsForAnimatedImages(doc, bgcolor);

		text.append(doc.generateHtml());
	}
	text.append("</font></p>");
	kdebugf2();
}

void History::showHistoryEntries(int from, int count)
{
	kdebugf();
	QString text;
	QStringList paracolors;
	unsigned int i;

	bool noStatus = config_file.readBoolEntry("History", "DontShowStatusChanges");

	QValueList<HistoryEntry> entries;
	entries = history.getHistoryEntries(uins, from, count);
	
	for (i = 0; i < entries.count(); ++i)
		if ( ! (noStatus && entries[i].type & HISTORYMANAGER_ENTRY_STATUS))
		{
			formatHistoryEntry(text, entries[i], paracolors);
			++i;
			break;
		}
	//z pierwszej wiadomości usuwamy obrazek separatora
	text.replace(QRegExp("<img title=\"\" height=\"[0-9]*\" width=\"10000\" align=\"right\">"), "");
	for (; i < entries.count(); ++i)
		if ( ! (noStatus && entries[i].type & HISTORYMANAGER_ENTRY_STATUS))
			formatHistoryEntry(text, entries[i], paracolors);

	body->setText(text);

	for (i = 0; i < paracolors.count(); ++i)
		body->setParagraphBackgroundColor(i, paracolors[i]);

	kdebugf2();
}

void History::searchBtnClicked()
{
	kdebugf();

	HistorySearch *hs;
	hs = new HistorySearch(this, uins);
//	hs->resetBtnClicked();
	hs->setDialogValues(findrec);
	if (hs->exec() == QDialog::Accepted)
	{
		findrec = hs->getDialogValues();
		findrec.actualrecord = -1;
		searchHistory();
	}
	delete hs;
	kdebugf2();
}

void History::searchNextBtnClicked()
{
	kdebugf();
	findrec.reverse = false;
	searchHistory();
	kdebugf2();
}

void History::searchPrevBtnClicked()
{
	kdebugf();
	findrec.reverse = true;
	searchHistory();
	kdebugf2();
}

QString History::gaduStatus2symbol(unsigned int status)
{
	switch (status)
	{
		case GG_STATUS_AVAIL:
		case GG_STATUS_AVAIL_DESCR:
			return QString("avail");
		case GG_STATUS_BUSY:
		case GG_STATUS_BUSY_DESCR:
			return QString("busy");
		case GG_STATUS_INVISIBLE:
		case GG_STATUS_INVISIBLE_DESCR:
			return QString("invisible");
		default:
			return QString("notavail");
	}
}

void History::setDateListViewText(QDateTime &datetime)
{
	kdebugf();
	QListViewItem *actlvi;
	actlvi = uinslv->firstChild();
	while (actlvi && !((UinsListViewText *)actlvi)->getUinsList().equals(uins))
		actlvi = actlvi->nextSibling();
	if (actlvi)
	{
		actlvi->setOpen(TRUE);
		actlvi = actlvi->firstChild();
		while (actlvi && ((DateListViewText *)actlvi)->getDate().date.date() != datetime.date())
			actlvi = actlvi->nextSibling();
		if (actlvi)
		{
			uinslv->setCurrentItem(actlvi);
//			body->setSelection(0, 0, 1, 10);
		}
	}
	kdebugf2();
}

void History::searchHistory()
{
	kdebugf();
	int start, end, count, total, len;
	unsigned int i;
	QDateTime fromdate, todate;
	QValueList<HistoryEntry> entries;
	QRegExp rxp;

	count = history.getHistoryEntriesCount(uins);
	if (findrec.fromdate.isNull())
		start = 0;
	else
		start = history.getHistoryEntryIndexByDate(uins, findrec.fromdate);
	if (findrec.todate.isNull())
		end = count - 1;
	else
		end = history.getHistoryEntryIndexByDate(uins, findrec.todate, true);
	kdebugmf(KDEBUG_INFO, "start = %d, end = %d\n", start, end);
	if (start > end || (start == end && (start == -1 || start == count)))
		return;
	if (start == -1)
		start = 0;
	if (end == count)
		--end;
	entries = history.getHistoryEntries(uins, start, 1);
	fromdate = entries[0].date;
	entries = history.getHistoryEntries(uins, end, 1);
	todate = entries[0].date;
	kdebugmf(KDEBUG_INFO, "start = %s, end = %s\n",
		fromdate.toString("dd.MM.yyyy hh:mm:ss").latin1(),
		todate.toString("dd.MM.yyyy hh:mm:ss").latin1());
	if (findrec.actualrecord == -1)
		findrec.actualrecord = findrec.reverse ? end : start;
	if ((findrec.actualrecord >= end && !findrec.reverse)
		|| (findrec.actualrecord <= start && findrec.reverse))
		return;
	if (findrec.reverse)
		total = findrec.actualrecord - start + 1;
	else
		total = end - findrec.actualrecord + 1;
	kdebugmf(KDEBUG_INFO, "findrec.type = %d\n", findrec.type);
	rxp.setPattern(findrec.data);
	setEnabled(false);
	finding = true;
	if (findrec.reverse)
		do
		{
			len = total > 100 ? 100 : total;
			entries = history.getHistoryEntries(uins, findrec.actualrecord - len + 1, len);
			for (i = 0; i < entries.count(); ++i)
				if ((findrec.type == 1 &&
					(entries[entries.count() - i - 1].type & HISTORYMANAGER_ENTRY_ALL_MSGS)
					&& entries[entries.count() - i - 1].message.contains(rxp)) ||
					(findrec.type == 2 &&
					(entries[entries.count() - i - 1].type & HISTORYMANAGER_ENTRY_STATUS)
					&& findrec.data == gaduStatus2symbol(entries[entries.count() - i - 1].status)))
				{
					setDateListViewText(entries[entries.count() - i - 1].date);
					//showHistoryEntries(findrec.actualrecord - i,
					//	findrec.actualrecord - i + 99 < count ? 100
					//	: count - findrec.actualrecord + i);
					History::start = findrec.actualrecord - i;
					break;
				}
			findrec.actualrecord -= i + (i < entries.count());
			total -= i + (i < entries.count());
			kdebugmf(KDEBUG_INFO, "actualrecord = %d, i = %d, total = %d\n",
				findrec.actualrecord, i, total);
			qApp->processEvents();
		} while (total > 0 && i == entries.count() && !closeDemand);
	else
		do
		{
			len = total > 100 ? 100 : total;
			entries = history.getHistoryEntries(uins, findrec.actualrecord, len);
			for (i = 0; i < entries.count(); ++i)
				if ((findrec.type == 1 && (entries[i].type & HISTORYMANAGER_ENTRY_ALL_MSGS)
					&& entries[i].message.contains(rxp)) ||
					(findrec.type == 2 &&
					(entries[i].type & HISTORYMANAGER_ENTRY_STATUS) &&
					findrec.data == gaduStatus2symbol(entries[i].status)))
				{
					setDateListViewText(entries[i].date);
					//showHistoryEntries(findrec.actualrecord + i,
					//	findrec.actualrecord + 99 < count ? 100
					//	: count - findrec.actualrecord - i);
					History::start = findrec.actualrecord + i;
					break;
				}
			findrec.actualrecord += i + (i < entries.count());
			total -= i + (i < entries.count());
			kdebugmf(KDEBUG_INFO, "actualrecord = %d, i = %d, total = %d\n",
				findrec.actualrecord, i, total);
			qApp->processEvents();
		} while (total > 0 && i == entries.count() && !closeDemand);
	if (closeDemand)
	{
		reject();
		kdebugf2();
		return;
	}
	if (findrec.actualrecord < 0)
		findrec.actualrecord = 0;
	setEnabled(true);
	finding = false;
	kdebugf2();
}

void History::closeEvent(QCloseEvent *e)
{
	saveGeometry(this, "History", "HistoryGeometry");

	if (finding)
	{
		e->ignore();
		closeDemand = true;
	}
	else
		e->accept();
}

void History::initModule()
{
	kdebugf();
	HistorySlots *historyslots=new HistorySlots(&history, "history_slots");

	//do usuniecia po wydaniu 0.4
	config_file.addVariable("History", "Logging", config_file.readEntry("General", "Logging"));
	//

	ConfigDialog::addTab(QT_TRANSLATE_NOOP("@default", "History"), "HistoryTab");
	ConfigDialog::addVGroupBox("History", "History", QT_TRANSLATE_NOOP("@default","Message citation in chat window"));
	ConfigDialog::addSpinBox("History", "Message citation in chat window", QT_TRANSLATE_NOOP("@default", "Count:"), "ChatHistoryCitation", 0, 200, 1, 10);
	ConfigDialog::addLabel("History", "Message citation in chat window", QT_TRANSLATE_NOOP("@default", "Don't cite messages older than:"));
	ConfigDialog::addSlider("History", "Message citation in chat window", "historyslider", "ChatHistoryQuotationTime", -744, -1, 24, -336);
	ConfigDialog::addLabel("History", "Message citation in chat window", "", "dayhour");
	ConfigDialog::addCheckBox("History", "History", QT_TRANSLATE_NOOP("@default", "Log messages"), "Logging", true);
	ConfigDialog::addCheckBox("History", "History", QT_TRANSLATE_NOOP("@default", "Don't show status changes"), "DontShowStatusChanges", false);
	ConfigDialog::addCheckBox("History", "History", QT_TRANSLATE_NOOP("@default", "Don't save status changes"), "DontSaveStatusChanges", true);

	ConfigDialog::registerSlotOnCreate(historyslots, SLOT(onCreateConfigDialog()));
	ConfigDialog::registerSlotOnApply(historyslots, SLOT(onDestroyConfigDialog()));
	ConfigDialog::connectSlot("History", "historyslider", SIGNAL(valueChanged(int)), historyslots, SLOT(updateQuoteTimeLabel(int)));

	connect(gadu,SIGNAL(chatMsgReceived1(UinsList,const QString&,time_t,bool&)),
		&history,SLOT(chatMsgReceived(UinsList,const QString&,time_t,bool&)));
	connect(gadu, SIGNAL(imageReceivedAndSaved(UinType, uint32_t, uint32_t, const QString &)),
		&history, SLOT(imageReceivedAndSaved(UinType, uint32_t, uint32_t, const QString &)));
	kdebugf2();
}

HistorySearch::HistorySearch(QWidget *parent, UinsList uins) : QDialog(parent), uins(uins)
{
	kdebugf();
	setCaption(tr("Search history"));

	int i;
	char buf[128];

	for (i = 0; i <= 59; ++i)
	{
		sprintf(buf, "%02d", i);
		numslist.append(QString(buf));
	}

	QStringList yearslist;
	for (i = 2000; i <= 2020; ++i)
		yearslist.append(QString::number(i));
	QStringList dayslist;
	for (i = 1; i <= 31; ++i)
		dayslist.append(numslist[i]);
	QStringList monthslist;
	for (i = 1; i <= 12; ++i)
		monthslist.append(numslist[i]);
	QStringList hourslist;
	for (i = 0; i <= 23; ++i)
		hourslist.append(numslist[i]);
	QStringList minslist;
	for (i = 0; i <= 59; ++i)
		minslist.append(numslist[i]);

	QHBox *from_hb = new QHBox(this);
	from_chb = new QCheckBox(tr("&From:") ,from_hb);
	from_hgb = new QHGroupBox(from_hb);
	from_day_cob = new QComboBox(from_hgb);
	from_day_cob->insertStringList(dayslist);
	QToolTip::add(from_day_cob, tr("day"));
	from_month_cob = new QComboBox(from_hgb);
	from_month_cob->insertStringList(monthslist);
	QToolTip::add(from_month_cob, tr("month"));
	from_year_cob = new QComboBox(from_hgb);
	from_year_cob->insertStringList(yearslist);
	QToolTip::add(from_year_cob, tr("year"));
	from_hour_cob = new QComboBox(from_hgb);
	from_hour_cob->insertStringList(hourslist);
	QToolTip::add(from_hour_cob, tr("hour"));
	from_min_cob = new QComboBox(from_hgb);
	from_min_cob->insertStringList(minslist);
	QToolTip::add(from_min_cob, tr("minute"));

	QHBox *to_hb = new QHBox(this);
	to_chb = new QCheckBox(tr("&To:") ,to_hb);
	to_hgb = new QHGroupBox(to_hb);
	to_day_cob = new QComboBox(to_hgb);
	to_day_cob->insertStringList(dayslist);
	QToolTip::add(to_day_cob, tr("day"));
	to_month_cob = new QComboBox(to_hgb);
	to_month_cob->insertStringList(monthslist);
	QToolTip::add(to_month_cob, tr("month"));
	to_year_cob = new QComboBox(to_hgb);
	to_year_cob->insertStringList(yearslist);
	QToolTip::add(to_year_cob, tr("year"));
	to_hour_cob = new QComboBox(to_hgb);
	to_hour_cob->insertStringList(hourslist);
	QToolTip::add(to_hour_cob, tr("hour"));
	to_min_cob = new QComboBox(to_hgb);
	to_min_cob->insertStringList(minslist);
	QToolTip::add(to_min_cob, tr("minute"));

	criteria_bg = new QVButtonGroup(tr("Find Criteria"), this);
	phrase_rb = new QRadioButton(tr("&Pattern"), criteria_bg);
	status_rb = new QRadioButton(tr("&Status"), criteria_bg);
	if (config_file.readBoolEntry("History", "DontShowStatusChanges"))
		status_rb->setEnabled(false);
	criteria_bg->insert(phrase_rb, 1);
	criteria_bg->insert(status_rb, 2);

	phrase_hgb = new QHGroupBox(tr("Pattern"), this);
	phrase_edit = new QLineEdit(phrase_hgb);
	status_hgb = new QHGroupBox(tr("Status"), this);
	status_cob = new QComboBox(status_hgb);
	for (i = 0; i < 4; ++i)
		status_cob->insertItem(qApp->translate("@default", UserStatus::name(i * 2)));

	reverse_chb = new QCheckBox(tr("&Reverse find"), this);

	QPushButton *find_btn = new QPushButton(tr("&Find"), this);
	QPushButton *reset_btn = new QPushButton(tr("Reset"), this);
	QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), this);

	connect(from_chb, SIGNAL(toggled(bool)), this, SLOT(fromToggled(bool)));
	connect(from_month_cob, SIGNAL(activated(int)), this, SLOT(correctFromDays(int)));
	connect(to_chb, SIGNAL(toggled(bool)), this, SLOT(toToggled(bool)));
	connect(to_month_cob, SIGNAL(activated(int)), this, SLOT(correctToDays(int)));
	connect(criteria_bg, SIGNAL(clicked(int)), this, SLOT(criteriaChanged(int)));
	connect(find_btn, SIGNAL(clicked()), this, SLOT(findBtnClicked()));
	connect(reset_btn, SIGNAL(clicked()), this, SLOT(resetBtnClicked()));
	connect(cancel_btn, SIGNAL(clicked()), this, SLOT(cancelBtnClicked()));

	QGridLayout *grid = new QGridLayout(this, 6, 4, 5, 5);
	grid->addMultiCellWidget(from_hb, 0, 0, 0, 3);
	grid->addMultiCellWidget(to_hb, 1, 1, 0, 3);
	grid->addMultiCellWidget(criteria_bg, 2, 3, 0, 1);
	grid->addMultiCellWidget(phrase_hgb, 2, 2, 2, 3);
	grid->addMultiCellWidget(status_hgb, 3, 3, 2, 3);
	grid->addMultiCellWidget(reverse_chb, 4, 4, 0, 3, Qt::AlignLeft);
	grid->addWidget(find_btn, 5, 1);
	grid->addWidget(reset_btn, 5, 2);
	grid->addWidget(cancel_btn, 5, 3);

	phrase_edit->setFocus();
	kdebugf2();
}

int daysForMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void HistorySearch::correctFromDays(int index)
{
	kdebugf();
	if (daysForMonth[index] != from_day_cob->count())
	{
		QStringList dayslist;
		for (int i = 1; i <= daysForMonth[index]; ++i)
			dayslist.append(numslist[i]);
		int current_day = from_day_cob->currentItem();
		from_day_cob->clear();
		from_day_cob->insertStringList(dayslist);
		if (current_day <= from_day_cob->count())
			from_day_cob->setCurrentItem(current_day);
	}
	kdebugf2();
}

void HistorySearch::correctToDays(int index)
{
	kdebugf();
	if (daysForMonth[index] != to_day_cob->count())
	{
		QStringList dayslist;
		for (int i = 1; i <= daysForMonth[index]; ++i)
			dayslist.append(numslist[i]);
		int current_day = to_day_cob->currentItem();
		to_day_cob->clear();
		to_day_cob->insertStringList(dayslist);
		if (current_day <= to_day_cob->count())
			to_day_cob->setCurrentItem(current_day);
	}
	kdebugf2();
}

void HistorySearch::fromToggled(bool on)
{
	from_hgb->setEnabled(on);
}

void HistorySearch::toToggled(bool on)
{
	to_hgb->setEnabled(on);
}

void HistorySearch::criteriaChanged(int id)
{
	phrase_hgb->setEnabled(id == 1);
	status_hgb->setEnabled(id != 1);
}

void HistorySearch::findBtnClicked()
{
	accept();
}

void HistorySearch::cancelBtnClicked()
{
	reject();
}

void HistorySearch::resetFromDate()
{
	kdebugf();
	QValueList<HistoryEntry> entries;

	entries = history.getHistoryEntries(uins, 0, 1);
	if (entries.count())
	{
		from_day_cob->setCurrentItem(entries[0].date.date().day() - 1);
		from_month_cob->setCurrentItem(entries[0].date.date().month() - 1);
		from_year_cob->setCurrentItem(entries[0].date.date().year() - 2000);
		from_hour_cob->setCurrentItem(entries[0].date.time().hour());
		from_min_cob->setCurrentItem(entries[0].date.time().minute());
		correctFromDays(entries[0].date.date().month() - 1);
	}
	kdebugf2();
}

void HistorySearch::resetToDate()
{
	kdebugf();
	QValueList<HistoryEntry> entries;

	entries = history.getHistoryEntries(uins, history.getHistoryEntriesCount(uins) - 1, 1);
	if (entries.count())
	{
		to_day_cob->setCurrentItem(entries[0].date.date().day() - 1);
		to_month_cob->setCurrentItem(entries[0].date.date().month() - 1);
		to_year_cob->setCurrentItem(entries[0].date.date().year() - 2000);
		to_hour_cob->setCurrentItem(entries[0].date.time().hour());
		to_min_cob->setCurrentItem(entries[0].date.time().minute());
		correctToDays(entries[0].date.date().month() - 1);
	}
	kdebugf2();
}

void HistorySearch::resetBtnClicked()
{
	kdebugf();
	from_hgb->setEnabled(false);
	from_chb->setChecked(false);
	resetFromDate();
	to_chb->setChecked(false);
	to_hgb->setEnabled(false);
	resetToDate();
	criteria_bg->setButton(1);
	phrase_edit->text().truncate(0);
	status_cob->setCurrentItem(0);
	criteriaChanged(1);
	reverse_chb->setChecked(false);
	kdebugf2();
}

void HistorySearch::setDialogValues(HistoryFindRec &findrec)
{
	kdebugf();
	from_chb->setChecked(!findrec.fromdate.isNull());
	from_hgb->setEnabled(!findrec.fromdate.isNull());
	if (findrec.fromdate.isNull())
		resetFromDate();
	else
	{
		from_day_cob->setCurrentItem(findrec.fromdate.date().day() - 1);
		from_month_cob->setCurrentItem(findrec.fromdate.date().month() - 1);
		from_year_cob->setCurrentItem(findrec.fromdate.date().year() - 2000);
		from_hour_cob->setCurrentItem(findrec.fromdate.time().hour());
		from_min_cob->setCurrentItem(findrec.fromdate.time().minute());
		correctFromDays(findrec.fromdate.date().month() - 1);
	}
	to_chb->setChecked(!findrec.todate.isNull());
	to_hgb->setEnabled(!findrec.todate.isNull());
	if (findrec.todate.isNull())
		resetToDate();
	else
	{
		to_day_cob->setCurrentItem(findrec.todate.date().day() - 1);
		to_month_cob->setCurrentItem(findrec.todate.date().month() - 1);
		to_year_cob->setCurrentItem(findrec.todate.date().year() - 2000);
		to_hour_cob->setCurrentItem(findrec.todate.time().hour());
		to_min_cob->setCurrentItem(findrec.todate.time().minute());
		correctToDays(findrec.todate.date().month() - 1);
	}
	criteria_bg->setButton(findrec.type);
	criteriaChanged(findrec.type);
	switch (findrec.type)
	{
		case 1:
			phrase_edit->setText(findrec.data);
			break;
		case 2:
		{
			int status=0;
			if (findrec.data == "avail")
				status = 0;
			else if (findrec.data == "busy")
				status = 1;
			else if (findrec.data == "invisible")
				status = 2;
			else if (findrec.data == "notavail")
				status = 3;
			status_cob->setCurrentItem(status);
			break;
		}
	}
	reverse_chb->setChecked(findrec.reverse);
	kdebugf2();
}

HistoryFindRec HistorySearch::getDialogValues()
{
	kdebugf();
	HistoryFindRec findrec;

	if (from_chb->isChecked())
	{
		findrec.fromdate.setDate(QDate(from_year_cob->currentItem() + 2000,
			from_month_cob->currentItem() + 1, from_day_cob->currentItem() + 1));
		findrec.fromdate.setTime(QTime(from_hour_cob->currentItem(), from_min_cob->currentItem()));
	}
	if (to_chb->isChecked())
	{
		findrec.todate.setDate(QDate(to_year_cob->currentItem() + 2000,
			to_month_cob->currentItem() + 1, to_day_cob->currentItem() + 1));
		findrec.todate.setTime(QTime(to_hour_cob->currentItem(), to_min_cob->currentItem()));
	}
	findrec.type = criteria_bg->id(criteria_bg->selected());
	switch (findrec.type)
	{
		case 1:
			findrec.data = phrase_edit->text();
			break;
		case 2:
			switch (status_cob->currentItem())
			{
				case 0:
					findrec.data = "avail";
					break;
				case 1:
					findrec.data = "busy";
					break;
				case 2:
					findrec.data = "invisible";
					break;
				case 3:
					findrec.data = "notavail";
					break;
			}
			break;
	}
	findrec.reverse = reverse_chb->isChecked();
	kdebugf2();
	return findrec;
}

HistorySlots::HistorySlots(QObject *parent, const char *name) : QObject(parent, name)
{
}

void HistorySlots::onCreateConfigDialog()
{
	kdebugf();
	QLabel *l_qtimeinfo=(QLabel*)(ConfigDialog::getWidget("History", "", "dayhour"));
	l_qtimeinfo->setAlignment(Qt::AlignHCenter);
	updateQuoteTimeLabel(config_file.readNumEntry("History", "ChatHistoryQuotationTime"));
	kdebugf2();
}

void HistorySlots::onDestroyConfigDialog()
{
//	kdebugf();
//	kdebugf2();
}

void HistorySlots::updateQuoteTimeLabel(int value)
{
	kdebugf();
	ConfigDialog::getLabel("History", "", "dayhour") ->
			setText(tr("%1 day(s) %2 hour(s)").arg(-value / 24).arg((-value) % 24));
	kdebugf2();
}

HistoryManager history(NULL, "history_manager");
