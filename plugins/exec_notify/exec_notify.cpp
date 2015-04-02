/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QProcess>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

#include "buddies/buddy-list.h"
#include "buddies/buddy.h"

#include "chat/chat.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"

#include "contacts/contact-set.h"

#include "gui/widgets/configuration/notify-group-box.h"
#include "gui/windows/main-configuration-window.h"

#include "misc/misc.h"

#include "notification/notification-manager.h"
#include "notification/notification/notification.h"

#include "core/application.h"
#include "icons/icons-manager.h"
#include "parser/parser.h"
#include "debug.h"

#include "exec_notify.h"

ExecConfigurationWidget::ExecConfigurationWidget(QWidget *parent)
	: NotifierConfigurationWidget(parent)
{
	commandLineEdit = new QLineEdit(this);
	commandLineEdit->setToolTip(QCoreApplication::translate("@default", MainConfigurationWindow::SyntaxTextNotify));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(commandLineEdit);

	static_cast<NotifyGroupBox *>(parent)->addWidget(this);
}

ExecConfigurationWidget::~ExecConfigurationWidget()
{
}

void ExecConfigurationWidget::saveNotifyConfigurations()
{
	if (!currentNotificationEvent.isEmpty())
		Commands[currentNotificationEvent] = commandLineEdit->text();

	for (QMap<QString, QString>::const_iterator it = Commands.constBegin(), end = Commands.constEnd(); it != end; ++it)
		Application::instance()->configuration()->deprecatedApi()->writeEntry("Exec Notify", it.key() + "Cmd", it.value());
}

void ExecConfigurationWidget::switchToEvent(const QString &event)
{
	if (!currentNotificationEvent.isEmpty())
		Commands[currentNotificationEvent] = commandLineEdit->text();
	currentNotificationEvent = event;

	if (Commands.contains(event))
		commandLineEdit->setText(Commands[event]);
	else
		commandLineEdit->setText(Application::instance()->configuration()->deprecatedApi()->readEntry("Exec Notify", event + "Cmd"));
}

ExecNotify::ExecNotify(QObject *parent) :
		Notifier("Exec", QT_TRANSLATE_NOOP("@default", "Run command"), KaduIcon("external_modules/execnotify"), parent)
{
	kdebugf();

	createDefaultConfiguration();
	NotificationManager::instance()->registerNotifier(this);

	kdebugf2();
}

ExecNotify::~ExecNotify()
{
	kdebugf();

	NotificationManager::instance()->unregisterNotifier(this);

	kdebugf2();
}

void ExecNotify::createDefaultConfiguration()
{
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "NewChatCmd", "Xdialog --msgbox \"#{protocol} %u %ids #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "NewMessageCmd", "Xdialog --msgbox \"#{protocol} %u %ids #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "ConnectionErrorCmd", "Xdialog --msgbox \"#{protocol} #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "StatusChanged", "Xdialog --msgbox \"#{protocol} %u #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "StatusChanged/ToFreeForChatCmd", "Xdialog --msgbox \"%protocol %u #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "StatusChanged/ToOnlineCmd", "Xdialog --msgbox \"%protocol %u #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "StatusChanged/ToAwayCmd", "Xdialog --msgbox \"#{protocol} %u #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "StatusChanged/ToNotAvailableCmd", "Xdialog --msgbox \"#{protocol} %u #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "StatusChanged/ToDoNotDisturbCmd", "Xdialog --msgbox \"#{protocol} %u #{event}\" 10 100");
	Application::instance()->configuration()->deprecatedApi()->addVariable("Exec Notify", "StatusChanged/ToOfflineCmd", "Xdialog --msgbox \"#{protocol} %u #{event}\" 10 100");
}

// TODO: merge with HistoryManager version
QStringList mySplit(const QChar &sep, const QString &str)
{
	kdebugf();
	QStringList strlist;
	QString token;
	int idx = 0, strlength = str.length();
	bool inString = false;

	int pos1, pos2;
	while (idx < strlength)
	{
		const QChar &letter = str[idx];
		if (inString)
		{
			if (letter == '\\')
			{
				switch (str[idx + 1].digitValue())
				{
					case 'n':
						token.append('\n');
						break;
					case '\\':
						token.append('\\');
						break;
					case '\"':
						token.append('"');
						break;
					default:
						token.append('?');
				}
				idx += 2;
			}
			else if (letter == '"')
			{
				strlist.append(token);
				inString = false;
				++idx;
			}
			else
			{
				pos1 = str.indexOf('\\', idx);
				if (pos1 == -1)
					pos1 = strlength;
				pos2 = str.indexOf('"', idx);
				if (pos2 == -1)
					pos2 = strlength;
				if (pos1 < pos2)
				{
					token.append(str.mid(idx, pos1 - idx));
					idx = pos1;
				}
				else
				{
					token.append(str.mid(idx, pos2 - idx));
					idx = pos2;
				}
			}
		}
		else // out of the string
		{
			if (letter == sep)
			{
				if (!token.isEmpty())
					token.clear();
				else
					strlist.append(QString());
			}
			else if (letter == '"')
				inString = true;
			else
			{
				pos1 = str.indexOf(sep, idx);
				if (pos1 == -1)
					pos1 = strlength;
				token.append(str.mid(idx, pos1 - idx));
				strlist.append(token);
				idx = pos1;
				continue;
			}
			++idx;
		}
	}

	kdebugf2();
	return strlist;
}

void ExecNotify::notify(Notification *notification)
{
	QString syntax = Application::instance()->configuration()->deprecatedApi()->readEntry("Exec Notify", notification->key() + "Cmd");
	if (syntax.isEmpty())
		return;
	QStringList s = mySplit(' ', syntax);
	QStringList result;

	auto chat = notification->data()["chat"].value<Chat>();
	if (chat)
	{
		ContactSet contacts = chat.contacts();

		QStringList sendersList;
		foreach (const Contact &contact, contacts)
			sendersList.append(Parser::escape(contact.id()));
		QString sendersString = sendersList.join(",");

		Contact contact = *contacts.constBegin();
		foreach (QString it, s)
			result.append(Parser::parse(it.replace("%ids", sendersString), Talkable(contact), notification, ParserEscape::HtmlEscape));
	}
	else
		foreach (const QString &it, s)
			result.append(Parser::parse(it, notification, ParserEscape::HtmlEscape));

	run(result);
}

void ExecNotify::run(const QStringList &args)
{
	foreach(const QString &arg, args)
	{
		kdebugm(KDEBUG_INFO, "arg: '%s'\n", qPrintable(arg));
	}

	QProcess *p = new QProcess(this);
	connect(p, SIGNAL(finished(int, QProcess::ExitStatus)), p, SLOT(deleteLater()));
	p->start(args.at(0), args.mid(1));
}

NotifierConfigurationWidget *ExecNotify::createConfigurationWidget(QWidget *parent)
{
	return new ExecConfigurationWidget(parent);
}

#include "moc_exec_notify.cpp"
