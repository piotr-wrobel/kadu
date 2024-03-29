/*
 * %kadu copyright begin%
 * Copyright 2013 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "buddy-configuration-widget-factory-repository.h"
#include "moc_buddy-configuration-widget-factory-repository.cpp"

BuddyConfigurationWidgetFactoryRepository::BuddyConfigurationWidgetFactoryRepository(QObject *parent) : QObject(parent)
{
}

BuddyConfigurationWidgetFactoryRepository::~BuddyConfigurationWidgetFactoryRepository()
{
}

void BuddyConfigurationWidgetFactoryRepository::registerFactory(BuddyConfigurationWidgetFactory *factory)
{
    if (Factories.contains(factory))
        return;

    Factories.append(factory);
    emit factoryRegistered(factory);
}

void BuddyConfigurationWidgetFactoryRepository::unregisterFactory(BuddyConfigurationWidgetFactory *factory)
{
    if (!Factories.contains(factory))
        return;

    Factories.removeAll(factory);
    emit factoryUnregistered(factory);
}

QList<BuddyConfigurationWidgetFactory *> BuddyConfigurationWidgetFactoryRepository::factories() const
{
    return Factories;
}
