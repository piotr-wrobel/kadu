/*
 * %kadu copyright begin%
 * Copyright 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "docking-tooltip-handler.h"
#include "moc_docking-tooltip-handler.cpp"

#include "docking-configuration-provider.h"
#include "docking-configuration.h"
#include "status-notifier-item.h"

#include "status/status-container-manager.h"
#include "status/status-type-data.h"
#include "status/status-type-manager.h"

DockingTooltipHandler::DockingTooltipHandler(QObject *parent) : QObject{parent}
{
}

DockingTooltipHandler::~DockingTooltipHandler()
{
}

void DockingTooltipHandler::setDockingConfigurationProvider(DockingConfigurationProvider *dockingConfigurationProvider)
{
    m_dockingConfigurationProvider = dockingConfigurationProvider;
}

void DockingTooltipHandler::setStatusContainerManager(StatusContainerManager *statusContainerManager)
{
    m_statusContainerManager = statusContainerManager;
}

void DockingTooltipHandler::setStatusNotifierItem(StatusNotifierItem *statusNotifierItem)
{
    m_statusNotifierItem = statusNotifierItem;
}

void DockingTooltipHandler::setStatusTypeManager(StatusTypeManager *statusTypeManager)
{
    m_statusTypeManager = statusTypeManager;
}

void DockingTooltipHandler::init()
{
    connect(m_dockingConfigurationProvider, SIGNAL(updated()), this, SLOT(updateTooltip()));
    connect(m_statusContainerManager, SIGNAL(statusUpdated(StatusContainer *)), this, SLOT(updateTooltip()));

    updateTooltip();
}

void DockingTooltipHandler::updateTooltip()
{
    m_statusNotifierItem->setTooltip(tooltip());
}

QString DockingTooltipHandler::tooltip() const
{
    if (!m_dockingConfigurationProvider->configuration().ShowTooltipInTray)
        return {};

    auto status = m_statusContainerManager->status();
    auto result = m_statusTypeManager->statusTypeData(status.type()).displayName();
    if (!status.description().isEmpty())
    {
        result += "\n";
        result += status.description();
    }

    return result;
}
