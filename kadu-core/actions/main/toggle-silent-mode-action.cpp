/*
 * %kadu copyright begin%
 * Copyright 2016 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "toggle-silent-mode-action.h"
#include "moc_toggle-silent-mode-action.cpp"

#include "actions/action.h"
#include "notification/silent-mode-service.h"

ToggleSilentModeAction::ToggleSilentModeAction(QObject *parent)
        :   // using C++ initializers breaks Qt's lupdate
          ActionDescription(parent)
{
    setCheckable(true);
    setIcon(KaduIcon{"kadu_icons/enable-notifications"});
    setName(QStringLiteral("silentModeAction"));
    setText(tr("Silent Mode"));
    setType(ActionDescription::TypeGlobal);
}

ToggleSilentModeAction::~ToggleSilentModeAction()
{
}

void ToggleSilentModeAction::setSilentModeService(SilentModeService *silentModeService)
{
    m_silentModeService = silentModeService;
}

void ToggleSilentModeAction::actionInstanceCreated(Action *action)
{
    action->setChecked(m_silentModeService->isSilent());
}

void ToggleSilentModeAction::actionTriggered(QAction *, bool toggled)
{
    m_silentModeService->setSilent(toggled);
}
