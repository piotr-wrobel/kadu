/*
 * %kadu copyright begin%
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>

#include "actions/action-context.h"
#include "actions/action.h"
#include "core/injected-factory.h"
#include "gui/status-icon.h"
#include "widgets/status-menu.h"

#include "change-status-action.h"
#include "moc_change-status-action.cpp"

ChangeStatusAction::ChangeStatusAction(QObject *parent) : ActionDescription(parent)
{
    setType(ActionDescription::TypeGlobal);
    setName("openStatusAction");
    setIcon(KaduIcon("kadu_icons/change-status"));
    setText(tr("Change Status"));
}

ChangeStatusAction::~ChangeStatusAction()
{
}

QMenu *ChangeStatusAction::menuForAction(Action *action)
{
    StatusContainer *container = action->context()->statusContainer();
    if (!container)
        return 0;

    // no parents for menu as it is destroyed manually by Action class
    auto menu = new QMenu();
    injectedFactory()->makeInjected<StatusMenu>(container, false, menu);
    return menu;
}

void ChangeStatusAction::actionInstanceCreated(Action *action)
{
    ActionDescription::actionInstanceCreated(action);

    StatusContainer *statusContainer = action->context()->statusContainer();
    if (statusContainer)
    {
        auto icon = injectedFactory()->makeInjected<StatusIcon>(statusContainer, action);
        connect(icon, SIGNAL(iconUpdated(KaduIcon)), action, SLOT(setIcon(KaduIcon)));
        action->setIcon(icon->icon());
    }
}
