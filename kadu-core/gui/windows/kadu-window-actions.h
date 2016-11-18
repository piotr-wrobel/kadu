/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

#include "status/status.h"

class Actions;
class AddConferenceAction;
class AddGroupAction;
class AddUserAction;
class AddRoomChatAction;
class ChangeStatusAction;
class ChatWidgetActions;
class CollapseAction;
class CopyDescriptionAction;
class CopyPersonalInfoAction;
class DeleteTalkableAction;
class DefaultProxyAction;
class EditTalkableAction;
class ExpandAction;
class ExitAction;
class LookupBuddyInfoAction;
class MenuInventory;
class MergeBuddiesAction;
class OpenBuddyEmailAction;
class OpenDescriptionLinkAction;
class OpenForumAction;
class OpenGetInvolvedAction;
class OpenRedmineAction;
class OpenSearchAction;
class OpenTranslateAction;
class RecentChatsAction;
class ShowAboutWindowAction;
class ShowBlockedBuddiesAction;
class ShowConfigurationWindowAction;
class ShowDescriptionsAction;
class ShowInfoPanelAction;
class ShowMultilogonsAction;
class ShowMyselfAction;
class ShowOfflineBuddiesAction;
class ShowOnlyBuddiesWithDescriptionAction;
class ShowOnlyBuddiesWithDescriptionOrOnlineAction;
class ShowYourAccountsAction;
class StatusContainer;

class KaduWindowActions : public QObject
{
	Q_OBJECT

	QPointer<Actions> m_actions;
	QPointer<AddConferenceAction> m_addConferenceAction;
	QPointer<AddGroupAction> m_addGroupAction;
	QPointer<AddRoomChatAction> m_addRoomChatAction;
	QPointer<AddUserAction> m_addUserAction;
	QPointer<ChatWidgetActions> m_chatWidgetActions;
	QPointer<ChangeStatusAction> m_changeStatusAction;
	QPointer<CollapseAction> m_collapseAction;
	QPointer<CopyDescriptionAction> m_copyDescriptionAction;
	QPointer<CopyPersonalInfoAction> m_copyPersonalInfoAction;
	QPointer<DefaultProxyAction> m_defaultProxyAction;
	QPointer<DeleteTalkableAction> m_deleteTalkableAction;
	QPointer<EditTalkableAction> m_editTalkableAction;
	QPointer<ExpandAction> m_expandAction;
	QPointer<ExitAction> m_exitAction;
	QPointer<LookupBuddyInfoAction> m_lookupBuddyInfoAction;
	QPointer<MenuInventory> m_menuInventory;
	QPointer<MergeBuddiesAction> m_mergeBuddiesAction;
	QPointer<OpenBuddyEmailAction> m_openBuddyEmailAction;
	QPointer<OpenDescriptionLinkAction> m_openDescriptionLinkAction;
	QPointer<OpenForumAction> m_openForumAction;
	QPointer<OpenGetInvolvedAction> m_openGetInvolvedAction;
	QPointer<OpenRedmineAction> m_openRedmineAction;
	QPointer<OpenSearchAction> m_openSearchAction;
	QPointer<OpenTranslateAction> m_openTranslateAction;
	QPointer<RecentChatsAction> m_recentChatsAction;
	QPointer<ShowAboutWindowAction> m_showAboutWindowAction;
	QPointer<ShowBlockedBuddiesAction> m_showBlockedBuddiesAction;
	QPointer<ShowConfigurationWindowAction> m_showConfigurationWindowAction;
	QPointer<ShowDescriptionsAction> m_showDescriptionsAction;
	QPointer<ShowInfoPanelAction> m_showInfoPanelAction;
	QPointer<ShowMultilogonsAction> m_showMultilogonsAction;
	QPointer<ShowMyselfAction> m_showMyselfAction;
	QPointer<ShowOfflineBuddiesAction> m_showOfflineBuddiesAction;
	QPointer<ShowOnlyBuddiesWithDescriptionAction> m_showOnlyBuddiesWithDescriptionAction;
	QPointer<ShowOnlyBuddiesWithDescriptionOrOnlineAction> m_showOnlyBuddiesWithDescriptionOrOnlineAction;
	QPointer<ShowYourAccountsAction> m_showYourAccountsAction;

	friend class KaduWindow;
	friend class TalkableTreeView;

private slots:
	INJEQT_SET void setActions(Actions *actions);
	INJEQT_SET void setAddConferenceAction(AddConferenceAction *addConferenceAction);
	INJEQT_SET void setAddGroupAction(AddGroupAction *addGroupAction);
	INJEQT_SET void setAddRoomChatAction(AddRoomChatAction *addRoomChatAction);
	INJEQT_SET void setAddUserAction(AddUserAction *addUserAction);
	INJEQT_SET void setChangeStatusAction(ChangeStatusAction *changeStatusAction);
	INJEQT_SET void setChatWidgetActions(ChatWidgetActions *chatWidgetActions);
	INJEQT_SET void setCollapseAction(CollapseAction *collapseAction);
	INJEQT_SET void setCopyDescriptionAction(CopyDescriptionAction *copyDescriptionAction);
	INJEQT_SET void setCopyPersonalInfoAction(CopyPersonalInfoAction *copyPersonalInfoAction);
	INJEQT_SET void setDefaultProxyAction(DefaultProxyAction *defaultProxyAction);
	INJEQT_SET void setDeleteTalkableAction(DeleteTalkableAction *deleteTalkableAction);
	INJEQT_SET void setEditTalkableAction(EditTalkableAction *editTalkableAction);
	INJEQT_SET void setExpandAction(ExpandAction *expandAction);
	INJEQT_SET void setExitAction(ExitAction *exitAction);
	INJEQT_SET void setLookupBuddyInfoAction(LookupBuddyInfoAction *lookupBuddyInfoAction);
	INJEQT_SET void setMenuInventory(MenuInventory *menuInventory);
	INJEQT_SET void setMergeBuddiesAction(MergeBuddiesAction *mergeBuddiesAction);
	INJEQT_SET void setOpenBuddyEmailAction(OpenBuddyEmailAction *openBuddyEmailAction);
	INJEQT_SET void setOpenDescriptionLinkAction(OpenDescriptionLinkAction *openDescriptionLinkAction);
	INJEQT_SET void setOpenForumAction(OpenForumAction *openForumAction);
	INJEQT_SET void setOpenGetInvolvedAction(OpenGetInvolvedAction *openGetInvolvedAction);
	INJEQT_SET void setOpenRedmineAction(OpenRedmineAction *openRedmineAction);
	INJEQT_SET void setOpenTranslateAction(OpenTranslateAction *openTranslateAction);
	INJEQT_SET void setOpenSearchAction(OpenSearchAction *openSearchAction);
	INJEQT_SET void setRecentChatsAction(RecentChatsAction *recentChatsAction);
	INJEQT_SET void setShowAboutWindowAction(ShowAboutWindowAction *showAboutWindowAction);
	INJEQT_SET void setShowBlockedBuddiesAction(ShowBlockedBuddiesAction *showBlockedBuddiesAction);
	INJEQT_SET void setShowConfigurationWindowAction(ShowConfigurationWindowAction *showConfigurationWindowAction);
	INJEQT_SET void setShowDescriptionsAction(ShowDescriptionsAction *showDescriptionsAction);
	INJEQT_SET void setShowInfoPanelAction(ShowInfoPanelAction *showInfoPanelAction);
	INJEQT_SET void setShowMultilogonsAction(ShowMultilogonsAction *showMultilogonsAction);
	INJEQT_SET void setShowMyselfAction(ShowMyselfAction *showMyselfAction);
	INJEQT_SET void setShowOfflineBuddiesAction(ShowOfflineBuddiesAction *showOfflineBuddiesAction);
	INJEQT_SET void setShowOnlyBuddiesWithDescriptionAction(ShowOnlyBuddiesWithDescriptionAction *showOnlyBuddiesWithDescriptionAction);
	INJEQT_SET void setShowOnlyBuddiesWithDescriptionOrOnlineAction(ShowOnlyBuddiesWithDescriptionOrOnlineAction *showOnlyBuddiesWithDescriptionOrOnlineAction);
	INJEQT_SET void setShowYourAccountsAction(ShowYourAccountsAction *showYourAccountsAction);
	INJEQT_INIT void init();
	INJEQT_DONE void done();

public:
	Q_INVOKABLE KaduWindowActions(QObject *parent = nullptr);
	virtual ~KaduWindowActions();

};

