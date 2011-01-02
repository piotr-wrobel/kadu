/*
 * %kadu copyright begin%
 * Copyright 2008, 2009, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2008 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2008, 2009 Piotr Galiszewski (piotrgaliszewski@gmail.com)
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

#ifndef KADU_MAIN_WINDOW_H
#define KADU_MAIN_WINDOW_H

#include <QtGui/QMainWindow>
#include <QtXml/QDomElement>

#include "gui/actions/action-data-source.h"
#include "gui/actions/action-description.h"
#include "gui/widgets/toolbar.h"

#include "exports.h"

class QContextMenuEvent;

class BuddiesListView;
class Buddy;
class BuddySet;
class Chat;
class Contact;
class ContactSet;
class StatusContainer;

class KADUAPI MainWindow : public QMainWindow, public ActionDataSource
{
	Q_OBJECT

	friend class Actions;

	QString WindowName;
	bool TransparencyEnabled;

	ToolBar *newToolbar(QWidget *parent);

protected:
	void loadToolBarsFromConfig();
	bool loadToolBarsFromConfig(const QString &configName, Qt::ToolBarArea area, bool remove = false);

	void writeToolBarsToConfig();
	void writeToolBarsToConfig(QDomElement parentConfig, const QString &configName, Qt::ToolBarArea area);

	static QDomElement getToolbarsConfigElement();
	static QDomElement getDockAreaConfigElement(QDomElement toolbarsConfig, const QString &name);
	static void addToolButton(QDomElement toolbarConfig, const QString &actionName, Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly);
	static QDomElement findExistingToolbarOnArea(const QString &areaName);
	static QDomElement findExistingToolbar(const QString &prefix);

	void setTransparency(bool enable);

	void contextMenuEvent(QContextMenuEvent *event);

protected slots:
	void refreshToolBars();

public:
	static MainWindow * findMainWindow(QWidget *widget);

	MainWindow(const QString &windowName, QWidget *parent);
	virtual ~MainWindow();

	QString windowName() { return WindowName; }

	virtual QMenu * createPopupMenu() { return 0; }

	virtual bool supportsActionType(ActionDescription::ActionType type) = 0;
	virtual BuddiesListView * buddiesListView() = 0;
	virtual StatusContainer * statusContainer() = 0;
	virtual Chat chat() = 0;
	Contact contact();
	Buddy buddy();

	void actionAdded(Action *action);

public slots:
	void addTopToolbar();
	void addBottomToolbar();
	void addLeftToolbar();
	void addRightToolbar();
	
};

#endif // KADU_MAIN_WINDOW_H
