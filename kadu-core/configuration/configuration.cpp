/*
 * %kadu copyright begin%
 * Copyright 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "configuration.h"
#include "moc_configuration.cpp"

#include "configuration/configuration-api.h"
#include "configuration/deprecated-configuration-api.h"
#include "misc/memory.h"

Configuration::Configuration(QString version, std::unique_ptr<ConfigurationApi> configurationApi, QObject *parent)
        : QObject{parent}, m_version{std::move(version)}, m_configurationApi{std::move(configurationApi)}
{
    m_deprecatedConfigurationApi =
        std::make_unique<DeprecatedConfigurationApi>(m_configurationApi.get(), QStringLiteral("kadu.conf"));
}

Configuration::~Configuration()
{
}

ConfigurationApi *Configuration::api() const
{
    return m_configurationApi.get();
}

DeprecatedConfigurationApi *Configuration::deprecatedApi() const
{
    return m_deprecatedConfigurationApi.get();
}

void Configuration::touch()
{
    m_configurationApi->touch(m_version);
}

QString Configuration::content() const
{
    return m_configurationApi->configuration();
}
