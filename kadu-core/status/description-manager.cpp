/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2010, 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "description-manager.h"
#include "moc_description-manager.cpp"

#include "configuration/configuration-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "status/description-model.h"

DescriptionManager::DescriptionManager(QObject *parent) : StorableStringList{parent}
{
}

DescriptionManager::~DescriptionManager()
{
}

void DescriptionManager::setConfigurationManager(ConfigurationManager *configurationManager)
{
    m_configurationManager = configurationManager;
}

void DescriptionManager::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void DescriptionManager::init()
{
    m_configurationManager->registerStorableObject(this);

    configurationUpdated();

    if (m_configuration->api()->getNode("Descriptions", ConfigurationApi::ModeFind).isNull())
        import();
    else
        setState(StateNotLoaded);
}

void DescriptionManager::done()
{
    m_configurationManager->unregisterStorableObject(this);
}

StorableObject *DescriptionManager::storageParent()
{
    return 0;
}

QString DescriptionManager::storageNodeName()
{
    return QStringLiteral("Descriptions");
}

QString DescriptionManager::storageItemNodeName()
{
    return QStringLiteral("Description");
}

DescriptionModel *DescriptionManager::model()
{
    ensureLoaded();

    return new DescriptionModel(this);
}

void DescriptionManager::import()
{
    StringList.clear();
    StringList.append(m_configuration->deprecatedApi()
                          ->readEntry("General", "DefaultDescription")
                          .split("<-->", QString::SkipEmptyParts));
    StringList.removeDuplicates();

    truncate();

    ensureStored();
}

void DescriptionManager::truncate()
{
    while (!StringList.isEmpty() && StringList.size() > MaxNumberOfDescriptions)
    {
        // We need a copy here, otherwise removeDescription() will emit singal descriptionRemoved()
        // with a reference to QString which will be already removed by then.
        QString last = StringList.last();
        removeDescription(last);
    }
}

void DescriptionManager::configurationUpdated()
{
    MaxNumberOfDescriptions = m_configuration->deprecatedApi()->readNumEntry("General", "NumberOfDescriptions");
    truncate();
}

void DescriptionManager::addDescription(const QString &description)
{
    if (description.isEmpty())
        return;

    if (StringList.contains(description))
        removeDescription(description);

    emit descriptionAboutToBeAdded(description);
    StringList.prepend(description);
    emit descriptionAdded(description);

    truncate();
}

void DescriptionManager::removeDescription(const QString &description)
{
    if (!StringList.contains(description))
        return;

    emit descriptionAboutToBeRemoved(description);
    StringList.removeAll(description);
    emit descriptionRemoved(description);
}

void DescriptionManager::clearDescriptions()
{
    StringList.clear();
}
