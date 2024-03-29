/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtWidgets/QApplication>

#include "actions/action-context.h"
#include "actions/action.h"
#include "buddies/buddy-set.h"
#include "chat/chat.h"
#include "chat/type/chat-type-manager.h"
#include "core/injected-factory.h"
#include "core/myself.h"
#include "icons/icons-manager.h"
#include "windows/buddy-delete-window.h"
#include "windows/kadu-window.h"
#include "windows/message-dialog.h"

#include "delete-talkable-action.h"
#include "moc_delete-talkable-action.cpp"

DeleteTalkableAction::DeleteTalkableAction(QObject *parent) : ActionDescription(parent)
{
    // TODO: TypeChat | TypeUser or TypeTalkables
    setType(TypeUser);
    setName("deleteUsersAction");
    setIcon(KaduIcon("edit-delete"));
    setShortcut("kadu_deleteuser");
    setText(tr("Delete Buddy"));
}

DeleteTalkableAction::~DeleteTalkableAction()
{
}

void DeleteTalkableAction::setChatTypeManager(ChatTypeManager *chatTypeManager)
{
    m_chatTypeManager = chatTypeManager;
}

void DeleteTalkableAction::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void DeleteTalkableAction::setMyself(Myself *myself)
{
    m_myself = myself;
}

int DeleteTalkableAction::actionRole(ActionContext *context) const
{
    if (context->roles().contains(ContactRole))   // we wont allow deleting contacts
        return -1;
    if (context->roles().contains(ChatRole))
        return ChatRole;
    if (context->roles().contains(BuddyRole))
        return BuddyRole;
    return -1;
}

Chat DeleteTalkableAction::actionChat(ActionContext *context) const
{
    return context->chat();
}

Buddy DeleteTalkableAction::actionBuddy(ActionContext *context) const
{
    if (context->buddies().size())
        return context->buddies().toBuddy();
    else
        return context->contacts().toContact().ownerBuddy();
}

void DeleteTalkableAction::setChatActionTitleAndIcon(Action *action)
{
    action->setText(QCoreApplication::translate("KaduWindowActions", "Delete Chat"));
}

void DeleteTalkableAction::setBuddyActionTitleAndIcon(Action *action)
{
    action->setText(QCoreApplication::translate("KaduWindowActions", "Delete Buddy"));
}

void DeleteTalkableAction::updateChatActionState(Action *action)
{
    setChatActionTitleAndIcon(action);

    auto const &chat = actionChat(action->context());
    auto chatType = m_chatTypeManager->chatType(chat.type());
    action->setEnabled(chat && (!chatType || (chatType->name() != "Contact" && !chat.display().isEmpty())));
}

void DeleteTalkableAction::updateBuddyActionState(Action *action)
{
    setBuddyActionTitleAndIcon(action);

    const BuddySet &buddies = action->context()->buddies();
    if (buddies.isEmpty() || buddies.contains(m_myself->buddy()))
        return;

    action->setEnabled(true);
}

void DeleteTalkableAction::actionInstanceCreated(Action *action)
{
    switch (actionRole(action->context()))
    {
    case ChatRole:
        setChatActionTitleAndIcon(action);
        break;
    case BuddyRole:
        setBuddyActionTitleAndIcon(action);
        break;
    }

    updateActionState(action);
}

void DeleteTalkableAction::updateActionState(Action *action)
{
    action->setEnabled(false);

    if (action->context()->buddies().isAnyTemporary())
        return;

    switch (actionRole(action->context()))
    {
    case ChatRole:
        updateChatActionState(action);
        break;
    case BuddyRole:
        updateBuddyActionState(action);
        break;
    }
}

void DeleteTalkableAction::chatActionTriggered(ActionContext *context)
{
    const Chat &chat = actionChat(context);
    if (!chat)
        return;

    MessageDialog *dialog = MessageDialog::create(
        m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Delete Chat"),
        tr("<b>%1</b> chat will be deleted.<br/>Are you sure?").arg(chat.display()));
    dialog->addButton(QMessageBox::Yes, tr("Delete chat"));
    dialog->addButton(QMessageBox::No, tr("Cancel"));

    if (dialog->ask())
        chat.setDisplay(QString());
}

void DeleteTalkableAction::buddyActionTriggered(ActionContext *context)
{
    auto buddySet = context->buddies();
    if (buddySet.empty())
        return;

    auto deleteWindow = injectedFactory()->makeInjected<BuddyDeleteWindow>(buddySet);
    deleteWindow->show();
}

void DeleteTalkableAction::triggered(QWidget *widget, ActionContext *context, bool toggled)
{
    Q_UNUSED(widget)
    Q_UNUSED(toggled)

    trigger(context);
}

void DeleteTalkableAction::trigger(ActionContext *context)
{
    switch (actionRole(context))
    {
    case ChatRole:
        chatActionTriggered(context);
        break;
    case BuddyRole:
        buddyActionTriggered(context);
        break;
    }
}
