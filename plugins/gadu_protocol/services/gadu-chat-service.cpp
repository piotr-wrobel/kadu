/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009, 2010 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2009, 2010, 2011, 2012, 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtCore/QScopedArrayPointer>
#include <QtCore/QTimer>

#include "chat/type/chat-type-contact-set.h"
#include "chat/type/chat-type-contact.h"
#include "configuration/configuration-file.h"
#include "contacts/contact-manager.h"
#include "contacts/contact-set.h"
#include "core/core.h"
#include "formatted-string/composite-formatted-string.h"
#include "formatted-string/formatted-string-factory.h"
#include "formatted-string/formatted-string-plain-text-visitor.h"
#include "gui/windows/message-dialog.h"
#include "services/image-storage-service.h"
#include "services/raw-message-transformer-service.h"
#include "status/status-type.h"
#include "debug.h"

#include "helpers/formatted-string-formats-visitor.h"
#include "helpers/formatted-string-image-key-received-visitor.h"
#include "helpers/gadu-formatter.h"
#include "helpers/gadu-protocol-helper.h"
#include "server/gadu-connection.h"
#include "server/gadu-writable-session-token.h"

#include "gadu-chat-service.h"

// TODO: move to const or something
#define MAX_DELIVERY_TIME 60 /*seconds*/
#define REMOVE_TIMER_INTERVAL 1000

GaduChatService::GaduChatService(Account account, QObject *parent) :
		ChatService(account, parent)
{
	RemoveTimer = new QTimer(this);
	RemoveTimer->setInterval(REMOVE_TIMER_INTERVAL);
	connect(RemoveTimer, SIGNAL(timeout()), this, SLOT(removeTimeoutUndeliveredMessages()));
	RemoveTimer->start();
}

GaduChatService::~GaduChatService()
{
}

void GaduChatService::setGaduChatImageService(GaduChatImageService *gaduChatImageService)
{
	CurrentGaduChatImageService = gaduChatImageService;
}

void GaduChatService::setImageStorageService(ImageStorageService *imageStorageService)
{
	CurrentImageStorageService = imageStorageService;
}

void GaduChatService::setFormattedStringFactory(FormattedStringFactory *formattedStringFactory)
{
	CurrentFormattedStringFactory = formattedStringFactory;
}

void GaduChatService::setConnection(GaduConnection *connection)
{
	Connection = connection;
}

int GaduChatService::maxMessageLength() const
{
	return 10000;
}

int GaduChatService::sendRawMessage(const QVector<Contact> &contacts, const QByteArray &rawMessage, const QByteArray &formats)
{
	if (!Connection || !Connection.data()->hasSession())
		return -1;

	auto writableSessionToken = Connection.data()->writableSessionToken();

	int messageId = -1;
	unsigned int uinsCount = contacts.count();
	if (uinsCount > 1)
	{
		QScopedArrayPointer<UinType> uins(contactsToUins(contacts));

		if (!formats.isEmpty())
			messageId = gg_send_message_confer_richtext(writableSessionToken.rawSession(), GG_CLASS_CHAT, uinsCount, uins.data(),
														(const unsigned char *) rawMessage.constData(), (const unsigned char *) formats.constData(), formats.size());
		else
			messageId = gg_send_message_confer(writableSessionToken.rawSession(), GG_CLASS_CHAT, uinsCount, uins.data(),
											   (const unsigned char *) rawMessage.constData());
	}
	else if (uinsCount == 1)
	{
		UinType uin = GaduProtocolHelper::uin(contacts.at(0));
		if (!formats.isEmpty())
			messageId = gg_send_message_richtext(writableSessionToken.rawSession(), GG_CLASS_CHAT, uin,
												 (const unsigned char *) rawMessage.constData(), (const unsigned char *) formats.constData(), formats.size());
		else
			messageId = gg_send_message(writableSessionToken.rawSession(), GG_CLASS_CHAT, uin,
										(const unsigned char *) rawMessage.constData());
	}

	return messageId;
}

QByteArray GaduChatService::formattedStringToFormats(FormattedString *formattedString) const
{
	FormattedStringFormatsVisitor formatsVisitor;
	formatsVisitor.setChatImageService(CurrentGaduChatImageService.data());
	formatsVisitor.setImageStorageService(CurrentImageStorageService.data());
	formattedString->accept(&formatsVisitor);

	return formatsVisitor.result();
}

UinType * GaduChatService::contactsToUins(const QVector<Contact> &contacts) const
{
	UinType * uins = new UinType[contacts.count()];
	unsigned int i = 0;

	foreach (const Contact &contact, contacts)
		uins[i++] = GaduProtocolHelper::uin(contact);

	return uins;
}

bool GaduChatService::sendMessage(const Message &message)
{
	if (!Connection || !Connection.data()->hasSession())
		return false;

	FormattedStringPlainTextVisitor plainTextVisitor;
	message.content()->accept(&plainTextVisitor);

	QByteArray rawMessage = plainTextVisitor.result().toUtf8();
	if (rawMessageTransformerService())
		rawMessage = rawMessageTransformerService()->transform(rawMessage, message);

	if (rawMessage.length() > maxMessageLength())
	{
		MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("Message too long (%1 >= %2)").arg(rawMessage.length()).arg(10000));
		kdebugmf(KDEBUG_FUNCTION_END, "end: filtered message too long\n");
		return false;
	}

	int messageId = sendRawMessage(message.messageChat().contacts().toContactVector(), rawMessage, formattedStringToFormats(message.content()));

	if (-1 == messageId)
		return false;

	message.setId(QString::number(messageId));
	UndeliveredMessages.insert(messageId, message);

	return true;
}

bool GaduChatService::sendRawMessage(const Chat &chat, const QByteArray &rawMessage)
{
	int messageId = sendRawMessage(chat.contacts().toContactVector(), rawMessage, QByteArray());
	return messageId != -1;
}

bool GaduChatService::isSystemMessage(gg_event *e)
{
	return 0 == e->event.msg.sender;
}

Contact GaduChatService::getSender(gg_event *e)
{
	return ContactManager::instance()->byId(account(), QString::number(e->event.msg.sender), ActionCreateAndAdd);
}

bool GaduChatService::ignoreSender(gg_event *e, Buddy sender)
{
	bool ignore =
			sender.isAnonymous() &&
			config_file.readBoolEntry("Chat", "IgnoreAnonymousUsers") &&
			(
				(e->event.msg.recipients_count == 0) ||
				config_file.readBoolEntry("Chat", "IgnoreAnonymousUsersInConferences")
			);

	return ignore;
}

ContactSet GaduChatService::getRecipients(gg_event *e)
{
	ContactSet recipients;
	for (int i = 0; i < e->event.msg.recipients_count; ++i)
		recipients.insert(ContactManager::instance()->byId(account(), QString::number(e->event.msg.recipients[i]), ActionCreateAndAdd));

	return recipients;
}

QByteArray GaduChatService::getRawContent(gg_event *e)
{
	return (const char *)e->event.msg.message;
}

bool GaduChatService::ignoreRichText(Contact sender)
{
	return sender.isAnonymous() && config_file.readBoolEntry("Chat","IgnoreAnonymousRichtext");
}

FormattedString * GaduChatService::createFormattedString(struct gg_event *e, const QString &content, bool richText)
{
	return CurrentFormattedStringFactory.data()->fromText(content);

	if (!richText)
		return GaduFormatter::createMessage(content, 0, 0);

	if (CurrentFormattedStringFactory.data()->isHtml(content))
		return CurrentFormattedStringFactory.data()->fromHtml(content);

	return GaduFormatter::createMessage(content, (unsigned char *)e->event.msg.formats, e->event.msg.formats_length);
}

void GaduChatService::handleMsg(Contact sender, ContactSet recipients, MessageType type, gg_event *e)
{
	ContactSet conference = recipients;
	conference += sender;

	ContactSet chatContacts = conference;
	chatContacts.remove(account().accountContact());

	if (chatContacts.isEmpty())
		return;

	Chat chat = 1 == chatContacts.size()
			? ChatTypeContact::findChat(*chatContacts.constBegin(), ActionCreateAndAdd)
			: ChatTypeContactSet::findChat(chatContacts, ActionCreateAndAdd);

	// create=true in our call for findChat(), but chat might be null for example if chatContacts was empty
	if (!chat || chat.isIgnoreAllMessages())
		return;

	Message message = Message::create();
	message.setMessageChat(chat);
	message.setType(type);
	message.setMessageSender(sender);
	message.setStatus(MessageTypeReceived == type ? MessageStatusReceived : MessageStatusSent);
	message.setSendDate(QDateTime::fromTime_t(e->event.msg.time));
	message.setReceiveDate(QDateTime::currentDateTime());

	QByteArray rawContent = getRawContent(e);
	if (rawMessageTransformerService())
		rawContent = rawMessageTransformerService()->transform(rawContent, message);

	QScopedPointer<FormattedString> formattedString(createFormattedString(e, QString::fromUtf8(rawContent), !ignoreRichText(sender)));
	if (formattedString->isEmpty())
		return;

	message.setContent(formattedString.take());

	if (MessageTypeReceived == type)
	{
		emit messageReceived(message);

		FormattedStringImageKeyReceivedVisitor imageKeyReceivedVisitor(sender.id());
		connect(&imageKeyReceivedVisitor, SIGNAL(chatImageKeyReceived(QString,ChatImageKey)),
		        this, SIGNAL(chatImageKeyReceived(QString,ChatImageKey)));

		message.content()->accept(&imageKeyReceivedVisitor);
	}
	else
		emit messageSent(message);
}

void GaduChatService::handleEventMsg(struct gg_event *e)
{
	kdebugmf(KDEBUG_NETWORK|KDEBUG_INFO, "recipients_count: %d\n", e->event.msg.recipients_count);

	if (isSystemMessage(e))
		return;

	if (GG_CLASS_CTCP == e->event.msg.msgclass) // old DCC6, not supported now
		return;

	Contact sender = getSender(e);
	if (ignoreSender(e, sender.ownerBuddy()))
		return;

	ContactSet recipients = getRecipients(e);

	handleMsg(sender, recipients, MessageTypeReceived, e);
}

void GaduChatService::handleEventMultilogonMsg(gg_event *e)
{
	// warning: this may be not intuitive code

	// we are sender
	Contact sender = account().accountContact();

	// e.sender + e.recipeints are real recipients
	ContactSet recipients = getRecipients(e);
	recipients.insert(getSender(e));

	handleMsg(sender, recipients, MessageTypeSent, e);
}

void GaduChatService::handleEventAck(struct gg_event *e)
{
	kdebugf();

	int messageId = e->event.ack.seq;
	if (!UndeliveredMessages.contains(messageId))
		return;

	UinType uin = e->event.ack.recipient;
	Q_UNUSED(uin) // only in debug mode

	switch (e->event.ack.status)
	{
		case GG_ACK_DELIVERED:
		case GG_ACK_QUEUED:
			kdebugm(KDEBUG_NETWORK|KDEBUG_INFO, "message delivered (uin: %u, seq: %d, status: %d)\n", uin, messageId, e->event.ack.status);
			UndeliveredMessages[messageId].setStatus(MessageStatusDelivered);
			emit sentMessageStatusChanged(UndeliveredMessages[messageId]);
			break;
		case GG_ACK_BLOCKED:
		case GG_ACK_MBOXFULL:
		case GG_ACK_NOT_DELIVERED:
			kdebugm(KDEBUG_NETWORK|KDEBUG_INFO, "message not delivered (uin: %u, seq: %d, status: %d)\n", uin, messageId, e->event.ack.status);
			UndeliveredMessages[messageId].setStatus(MessageStatusWontDeliver);
			emit sentMessageStatusChanged(UndeliveredMessages[messageId]);
			break;
		default:
			kdebugm(KDEBUG_NETWORK|KDEBUG_WARNING, "unknown acknowledge! (uin: %u, seq: %d, status:%d)\n", uin, messageId, e->event.ack.status);
			return;
	}
	UndeliveredMessages.remove(messageId);

	removeTimeoutUndeliveredMessages();

	kdebugf2();
}

void GaduChatService::removeTimeoutUndeliveredMessages()
{
	QDateTime now = QDateTime::currentDateTime();
	QHash<int, Message>::iterator it = UndeliveredMessages.begin();
	QVector<Message> removedMessages;

	while (it != UndeliveredMessages.end())
		if (it.value().sendDate().addSecs(MAX_DELIVERY_TIME) < now)
		{
			removedMessages.append(it.value());
			it = UndeliveredMessages.erase(it);
		}
		else
			++it;

	foreach (const Message &message, removedMessages)
	{
		message.setStatus(MessageStatusWontDeliver);
		emit sentMessageStatusChanged(message);
	}
}

#include "moc_gadu-chat-service.cpp"
