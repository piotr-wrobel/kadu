/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "contacts/contact-manager.h"
#include "contacts/group-manager.h"
#include "pending_msgs.h"
#include "xml_config_file.h"

#include "configuration-manager.h"

ConfigurationManager::ConfigurationManager()
{

}

void ConfigurationManager::load()
{
	xml_config_file->makeBackup();

	pending.loadConfiguration(xml_config_file);

	GroupManager::instance()->loadConfiguration();
	ContactManager::instance()->loadConfiguration();
}

void ConfigurationManager::store()
{

}
