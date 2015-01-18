/*
 * %kadu copyright begin%
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2004 Adrian Smarzewski (adrian@kadu.net)
 * Copyright 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2012 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2004, 2006 Marcin Ślusarz (joi@kadu.net)
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

#include "window-chat-widget-container-handler.h"

#include "gui/widgets/chat-widget/chat-widget-manager.h"
#include "gui/widgets/chat-widget/chat-widget.h"
#include "gui/windows/chat-window/chat-window-factory.h"
#include "gui/windows/chat-window/chat-window-repository.h"
#include "gui/windows/chat-window/chat-window.h"
#include "activate.h"

#include <QtWidgets/QApplication>

WindowChatWidgetContainerHandler::WindowChatWidgetContainerHandler(QObject *parent) :
		ChatWidgetContainerHandler{parent}
{
}

WindowChatWidgetContainerHandler::~WindowChatWidgetContainerHandler()
{
}

void WindowChatWidgetContainerHandler::setChatWindowFactory(ChatWindowFactory *chatWindowFactory)
{
	m_chatWindowFactory = chatWindowFactory;
}

void WindowChatWidgetContainerHandler::setChatWindowRepository(ChatWindowRepository *chatWindowRepository)
{
	m_chatWindowRepository = chatWindowRepository;
}

bool WindowChatWidgetContainerHandler::acceptChat(Chat chat) const
{
	return chat && m_chatWindowFactory && m_chatWindowRepository;
}

void WindowChatWidgetContainerHandler::addChatWidget(ChatWidget *chatWidget)
{
	if (!chatWidget || !m_chatWindowFactory || !m_chatWindowRepository)
		return;

	auto chatWindow = m_chatWindowRepository.data()->windowForChat(chatWidget->chat());
	if (!chatWindow)
	{
		auto newChatWindow = m_chatWindowFactory.data()->createChatWindow(chatWidget);
		chatWindow = newChatWindow.get();
		if (!chatWindow)
			return;

		m_chatWindowRepository.data()->addChatWindow(std::move(newChatWindow));
		connect(chatWindow, SIGNAL(activated(ChatWindow*)), this, SLOT(chatWindowActivated(ChatWindow*)));
	}


	switch (chatWidget->activation())
	{
		case OpenChatActivation::Minimize:
			chatWindow->showMinimized();
			break;
		default:
			chatWindow->show();
			break;
	}

	if (chatWidget->unreadMessagesCount() != 0)
		qApp->alert(chatWindow);

	chatWidget->setActivation(OpenChatActivation::Ignore);
}

void WindowChatWidgetContainerHandler::removeChatWidget(ChatWidget *chatWidget)
{
	if (!chatWidget || !m_chatWindowRepository)
		return;

	auto chatWindow = m_chatWindowRepository.data()->windowForChat(chatWidget->chat());
	m_chatWindowRepository.data()->removeChatWindow(chatWindow);
}

bool WindowChatWidgetContainerHandler::isChatWidgetActive(ChatWidget *chatWidget)
{
	if (!chatWidget || !m_chatWindowRepository)
		return false;

	auto chatWindow = m_chatWindowRepository.data()->windowForChat(chatWidget->chat());
	return chatWindow ? chatWindow->isChatWidgetActive(chatWidget) : false;
}

void WindowChatWidgetContainerHandler::tryActivateChatWidget(ChatWidget *chatWidget)
{
	if (!chatWidget || !m_chatWindowRepository)
		return;

	auto chatWindow = m_chatWindowRepository.data()->windowForChat(chatWidget->chat());
	if (chatWindow)
		_activateWindow(chatWindow);
}

void WindowChatWidgetContainerHandler::tryMinimizeChatWidget(ChatWidget* chatWidget)
{
	if (!chatWidget || !m_chatWindowRepository)
		return;

	auto chatWindow = m_chatWindowRepository.data()->windowForChat(chatWidget->chat());
	if (chatWindow)
		chatWindow->showMinimized();
}

void WindowChatWidgetContainerHandler::chatWindowActivated(ChatWindow *chatWindow)
{
	if (chatWindow && chatWindow->chatWidget())
		emit chatWidgetActivated(chatWindow->chatWidget());
}

#include "moc_window-chat-widget-container-handler.cpp"
