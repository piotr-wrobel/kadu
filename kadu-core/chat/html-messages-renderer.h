/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#ifndef HTML_MESSAGES_RENDERER_H
#define HTML_MESSAGES_RENDERER_H

#include <QtCore/QObject>

#include "chat/chat.h"
#include "message/message.h"
#include "protocols/services/chat-state-service.h"

class QWebPage;

class Chat;
class ChatImageKey;
class MessageRenderInfo;

class HtmlMessagesRenderer : public QObject
{
	Q_OBJECT

	Chat MyChat;
	QList<MessageRenderInfo *> MyChatMessages;
	MessageRenderInfo *LastMessage;
	QWebPage *MyWebPage;

	bool PruneEnabled;
	bool ForcePruneDisabled;

	void pruneMessages();

public:
	explicit HtmlMessagesRenderer(const Chat &chat, QObject *parent = 0);
	virtual ~HtmlMessagesRenderer();

	Chat  chat() { return MyChat; }
	void setChat(const Chat &chat);

	QWebPage * webPage() { return MyWebPage; }

	bool pruneEnabled();

	bool forcePruneDisabled() { return ForcePruneDisabled; }
	void setForcePruneDisabled(bool forcePruneDisabled);

	QString content();

	const QList<MessageRenderInfo *> & messages() const { return MyChatMessages; }
	void appendMessage(MessageRenderInfo *message);
	void appendMessages(const QList<MessageRenderInfo *> &messages);
	void clearMessages();

	MessageRenderInfo * lastMessage() { return LastMessage; }
	void setLastMessage(MessageRenderInfo *message);

	void refresh();
	void chatImageAvailable(const ChatImageKey &imageKey, const QString &fileName);
	void updateBackgroundsAndColors();
	void messageStatusChanged(Message message, MessageStatus status);
	void contactActivityChanged(const Contact &contact, ChatStateService::State state);

};

#endif // HTML_MESSAGES_RENDERER_H
