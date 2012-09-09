/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2009 Longer (longer89@gmail.com)
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

#ifndef MENU_INVENTORY_H
#define MENU_INVENTORY_H

#include <QtCore/QObject>

#include "kadu-menu.h"
#include "exports.h"

class QMenu;
class QWidget;

class ActionContext;
class ActionDescription;
class Contact;
class ProtocolMenuManager;

class KADUAPI MenuInventory : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(MenuInventory)

	static MenuInventory *Instance;

	QMap<KaduMenu::MenuCategory, KaduMenu *> Menus;
	QList<ProtocolMenuManager *> ProtocolMenuManagers;

	MenuInventory();

public:
	static MenuInventory * instance();

	void bindMenu(KaduMenu::MenuCategory, QMenu *menu);
	KaduMenu * menu(KaduMenu::MenuCategory category);

	void registerProtocolMenuManager(ProtocolMenuManager *manager);
	void unregisterProtocolMenuManager(ProtocolMenuManager *manager);

	QList<ProtocolMenuManager *> & protocolMenuManagers() { return ProtocolMenuManagers; }
};

#endif // MENU_INVENTORY_H
