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

#include "docking-configuration-provider.h"
#include "moc_docking-configuration-provider.cpp"

#include "docking-configuration.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"

DockingConfigurationProvider::DockingConfigurationProvider(QObject *parent) : QObject{parent}
{
}

DockingConfigurationProvider::~DockingConfigurationProvider()
{
}

void DockingConfigurationProvider::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void DockingConfigurationProvider::init()
{
    createDefaultConfiguration();
    configurationUpdated();
}

void DockingConfigurationProvider::createDefaultConfiguration()
{
    m_configuration->deprecatedApi()->addVariable("General", "RunDocked", false);
    m_configuration->deprecatedApi()->addVariable("General", "ShowTooltipInTray", true);
    m_configuration->deprecatedApi()->addVariable("Look", "NewMessageIcon", 0);
}

const DockingConfiguration &DockingConfigurationProvider::configuration() const
{
    return m_dockingConfiguration;
}

void DockingConfigurationProvider::configurationUpdated()
{
    m_dockingConfiguration =
        DockingConfiguration{m_configuration->deprecatedApi()->readBoolEntry("General", "RunDocked"),
                             m_configuration->deprecatedApi()->readBoolEntry("General", "ShowTooltipInTray"),
                             static_cast<StatusNotifierItemAttentionMode>(
                                 m_configuration->deprecatedApi()->readNumEntry("Look", "NewMessageIcon"))};

    emit updated();
}
