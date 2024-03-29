/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2008, 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "core/core.h"

#include "config-file-data-manager.h"
#include "moc_config-file-data-manager.cpp"

void ConfigFileDataManager::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void ConfigFileDataManager::writeEntry(const QString &section, const QString &name, const QVariant &value)
{
    if (section.isEmpty() || name.isEmpty())
        return;

    m_configuration->deprecatedApi()->writeEntry(section, name, value.toString());
}

QVariant ConfigFileDataManager::readEntry(const QString &section, const QString &name)
{
    if (section.isEmpty() || name.isEmpty())
        return QVariant(QString());

    return QVariant(m_configuration->deprecatedApi()->readEntry(section, name));
}
