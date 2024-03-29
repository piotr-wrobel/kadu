/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "leave-chat-action.h"
#include "moc_leave-chat-action.cpp"

#include "actions/action-context.h"
#include "chat/chat-service-repository.h"
#include "chat/chat.h"
#include "icons/icons-manager.h"
#include "protocols/services/chat-service.h"
#include "widgets/chat-widget/chat-widget-manager.h"
#include "widgets/chat-widget/chat-widget-repository.h"
#include "widgets/chat-widget/chat-widget.h"
#include "windows/message-dialog.h"

LeaveChatAction::LeaveChatAction(QObject *parent) : ActionDescription(parent)
{
    setType(ActionDescription::TypeChat);
    setName("leaveChatAction");
    setIcon(KaduIcon("kadu_icons/block-buddy"));
    setText(tr("Leave"));
}

LeaveChatAction::~LeaveChatAction()
{
}

void LeaveChatAction::setChatServiceRepository(ChatServiceRepository *chatServiceRepository)
{
    m_chatServiceRepository = chatServiceRepository;
}

void LeaveChatAction::setChatWidgetRepository(ChatWidgetRepository *chatWidgetRepository)
{
    m_chatWidgetRepository = chatWidgetRepository;
}

void LeaveChatAction::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void LeaveChatAction::triggered(QWidget *widget, ActionContext *context, bool toggled)
{
    Q_UNUSED(toggled)

    auto chat = context->chat();
    if (!chat)
        return;

    auto chatService = m_chatServiceRepository->chatService(chat.chatAccount());
    if (!chatService)
        return;

    auto dialog = MessageDialog::create(
        m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Kadu"),
        tr("All messages received in this conference will be ignored\nfrom now on. Are you sure you want to leave this "
           "conference?"),
        widget);
    dialog->addButton(QMessageBox::Yes, tr("Leave conference"));
    dialog->addButton(QMessageBox::No, tr("Cancel"));
    if (!dialog->ask())
        return;

    chatService->leaveChat(chat);

    auto chatWidget = m_chatWidgetRepository->widgetForChat(chat);
    if (chatWidget)
        chatWidget->requestClose();
}
