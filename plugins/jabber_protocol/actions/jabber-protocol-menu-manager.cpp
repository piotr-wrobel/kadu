/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
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

#include "jabber-protocol-menu-manager.h"

#include "actions/ask-for-subscription-action.h"
#include "actions/jabber-actions.h"
#include "actions/remove-subscription-action.h"
#include "actions/resend-subscription-action.h"

#include "gui/actions/actions.h"

JabberProtocolMenuManager::JabberProtocolMenuManager(QObject *parent) :
		QObject{parent}
{
}

JabberProtocolMenuManager::~JabberProtocolMenuManager()
{
	m_rosterActions.clear();
}

void JabberProtocolMenuManager::setActions(Actions *actions)
{
	m_actions = actions;
}

void JabberProtocolMenuManager::setAskForSubscriptionAction(AskForSubscriptionAction *askForSubscriptionAction)
{
	m_askForSubscriptionAction = askForSubscriptionAction;
}

void JabberProtocolMenuManager::setJabberActions(JabberActions *jabberActions)
{
	m_jabberActions = jabberActions;
}

void JabberProtocolMenuManager::setRemoveSubscriptionAction(RemoveSubscriptionAction *removeSubscriptionAction)
{
	m_removeSubscriptionAction = removeSubscriptionAction;
}

void JabberProtocolMenuManager::setResendSubscriptionAction(ResendSubscriptionAction *resendSubscriptionAction)
{
	m_resendSubscriptionAction = resendSubscriptionAction;
}

void JabberProtocolMenuManager::init()
{
	m_actions->insert(m_askForSubscriptionAction);
	m_actions->insert(m_removeSubscriptionAction);
	m_actions->insert(m_resendSubscriptionAction);
}

void JabberProtocolMenuManager::done()
{
	m_actions->remove(m_askForSubscriptionAction);
	m_actions->remove(m_removeSubscriptionAction);
	m_actions->remove(m_resendSubscriptionAction);
}

const QList<ActionDescription *> & JabberProtocolMenuManager::protocolActions() const
{
	if (m_rosterActions.empty())
	{
		m_rosterActions.append(m_resendSubscriptionAction);
		m_rosterActions.append(m_removeSubscriptionAction);
		m_rosterActions.append(m_askForSubscriptionAction);
	}

	return m_rosterActions;
}

#include "moc_jabber-protocol-menu-manager.cpp"
