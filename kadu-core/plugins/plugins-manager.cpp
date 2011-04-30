/*
 * %kadu copyright begin%
 * Copyright 2010 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2007, 2008 Dawid Stawiarski (neeo@kadu.net)
 * Copyright 2010, 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2008, 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2004, 2005, 2006, 2007 Marcin Ślusarz (joi@kadu.net)
 * Copyright 2003, 2004, 2005 Adrian Smarzewski (adrian@kadu.net)
 * Copyright 2007, 2008, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2004 Roman Krzystyniak (Ron_K@tlen.pl)
 * Copyright 2004, 2008, 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2008, 2009, 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2010 Radosław Szymczyszyn (lavrin@gmail.com)
 * Copyright 2004, 2005 Paweł Płuciennik (pawel_p@kadu.net)
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

#include <QtCore/QDir>
#include <QtCore/QLibrary>
#include <QtCore/QPluginLoader>
#include <QtCore/QTextCodec>
#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>

#ifndef Q_WS_WIN
#include <dlfcn.h>
#endif

#include "configuration/configuration-file.h"
#include "configuration/configuration-manager.h"
#include "core/core.h"
#include "gui/hot-key.h"
#include "gui/windows/kadu-window.h"
#include "gui/windows/modules-window.h"
#include "gui/windows/message-dialog.h"
#include "misc/path-conversion.h"
#include "plugins/generic-plugin.h"
#include "plugins/plugin.h"
#include "plugins/plugin-info.h"
#include "activate.h"
#include "debug.h"
#include "icons/icons-manager.h"

#include "plugins-manager.h"

#ifdef Q_OS_MAC
	#define SO_EXT "so"
	#define SO_EXT_LEN 2
	#define SO_PREFIX "lib"
	#define SO_PREFIX_LEN 3
#elif defined(Q_OS_WIN)
	#define SO_EXT "dll"
	#define SO_EXT_LEN 3
	#define SO_PREFIX ""
	#define SO_PREFIX_LEN 0
#else
	#define SO_EXT "so"
	#define SO_EXT_LEN 2
	#define SO_PREFIX "lib"
	#define SO_PREFIX_LEN 3
#endif

PluginsManager * PluginsManager::Instance = 0;

PluginsManager * PluginsManager::instance()
{
	if (0 == Instance)
	{
		Instance = new PluginsManager();
		// do not move to contructor
		// Instance variable must be available ModulesManager::load method
		Instance->ensureLoaded();
	}

	return Instance;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Creates new PluginsManager and registers it in ConfigurationManager singleton.
 *
 * Creates new PluginsManager, registers it in ConfigurationManager singleton.
 * Storage status is set to Storage::StateNotLoaded.
 */
PluginsManager::PluginsManager() :
		Plugins(), Window(0)
{
	ConfigurationManager::instance()->registerStorableObject(this);

	setState(StateNotLoaded);
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Destroys instance and unregisters it from ConfigurationManager singleton.
 *
 * Destroys instance and unregisters it from ConfigurationManager singleton.
 */
PluginsManager::~PluginsManager()
{
	ConfigurationManager::instance()->unregisterStorableObject(this);
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Loads PluginsManager and all Plugin configurations.
 *
 * This method loads PluginsManager configuration from storage node /root/Plugins and all sub nodes
 * /root/Plugins/Plugin. If attribute /root/Plugins/\@imported_from_09 is not present importFrom09()
 * method will be called to import depreceated configuration from 0.9.x and earlier versions.
 *
 * After reading all plugins configuration this method check for existence of new plugins that could
 * be recently installed. Check is done by searching datadir/kadu/plugins directory for new *.desc
 * files. All new plugins are set to have Plugin::PluginStateNew state.
 */
void PluginsManager::load()
{
	if (!isValidStorage())
		return;

	StorableObject::load();

	QDomElement itemsNode = storage()->point();
	if (!itemsNode.isNull())
	{
		QList<QDomElement> pluginElements = storage()->storage()->getNodes(itemsNode, QLatin1String("Plugin"));

		foreach (const QDomElement &pluginElement, pluginElements)
		{
			QSharedPointer<StoragePoint> storagePoint(new StoragePoint(storage()->storage(), pluginElement));
			QString name = storagePoint->point().attribute("name");
			if (!name.isEmpty() && !Plugins.contains(name))
			{
				Plugin *plugin = new Plugin(name, this);
				Plugins.insert(name, plugin);
			}
		}
	}

	foreach (const QString &moduleName, installedPlugins())
		if (!Plugins.contains(moduleName))
		{
			Plugin *plugin = new Plugin(moduleName, this);
			Plugins.insert(moduleName, plugin);
		}

	if (!loadAttribute<bool>("imported_from_09", false))
	{
		importFrom09();
		storeAttribute("imported_from_09", true);
	}
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Stores PluginsManager and all Plugin configurations.
 *
 * This method stores PluginsManager configuration to storage node /root/Plugins and all sub nodes to
 * /root/Plugins/Plugin. Attribute /root/Plugins/\@imported_from_09 is always stored as "true".
 */
void PluginsManager::store()
{
	if (!isValidStorage())
		return;

	ensureLoaded();

	StorableObject::store();

	foreach (Plugin *plugin, Plugins)
		plugin->store();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Import 0.9.x configuration.
 *
 * This method loads old configuration from depreceated configuration entries: General/EverLaoded,
 * General/LoadedModules and General/UnloadedModules. Do not call it manually.
 */
void PluginsManager::importFrom09()
{
	QStringList everLoaded = config_file.readEntry("General", "EverLoaded").split(',', QString::SkipEmptyParts);
	QString loaded = config_file.readEntry("General", "LoadedModules");

	QStringList loadedPlugins = loaded.split(',', QString::SkipEmptyParts);
	everLoaded += loadedPlugins;
	QString unloaded_str = config_file.readEntry("General", "UnloadedModules");
	QStringList unloadedPlugins = unloaded_str.split(',', QString::SkipEmptyParts);

	QStringList allPlugins = everLoaded + unloadedPlugins; // just in case...
	foreach (const QString &pluginName, allPlugins)
		if (!Plugins.contains(pluginName))
		{
			Plugin *plugin = new Plugin(pluginName, this);
			Plugins.insert(pluginName, plugin);
		}

	if (loadedPlugins.contains("encryption"))
	{
		loadedPlugins.removeAll("encryption");
		loadedPlugins.append("encryption_ng");
		loadedPlugins.append("encryption_ng_simlite");
	}
	if (loadedPlugins.contains("osd_hints"))
	{
		loadedPlugins.removeAll("osd_hints");
		if (!loadedPlugins.contains("hints"))
			loadedPlugins.append("hints");
	}

	ensureLoadedAtLeastOnce("gadu_protocol");
	ensureLoadedAtLeastOnce("jabber_protocol");
	ensureLoadedAtLeastOnce("sql_history");
	ensureLoadedAtLeastOnce("history_migration");
	ensureLoadedAtLeastOnce("profiles_import");

	foreach (Plugin *plugin, Plugins)
		if (allPlugins.contains(plugin->name()))
		{
			if (loadedPlugins.contains(plugin->name()))
				plugin->setState(Plugin::PluginStateEnabled);
			else
				plugin->setState(Plugin::PluginStateDisabled);
		}
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Enure that given plugin was/will be activated at least once.
 * @param pluginName name of plugin to check
 *
 * This method is used to fix broken configurations that had importand modules marked
 * as unloaded without even loading them one time. Do not call this method, it is used
 * internally by importFrom09().
 */
void PluginsManager::ensureLoadedAtLeastOnce(const QString& pluginName)
{
	if (!Plugins.contains(pluginName))
		return;

	if (!Plugin::PluginStateNew == Plugins.value(pluginName)->state())
		Plugins.value(pluginName)->setState(Plugin::PluginStateEnabled);
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Activate all protocols plugins that are enabled.
 *
 * This method activates all plugins with type "protocol" that are either enabled (Plugin::PluginStateEnabled)
 * or new (Plugin::PluginStateNew) with attribute "load by default" set. This method is generally called before
 * any other activation to ensure that all protocols and accounts are available for other plugins.
 */
void PluginsManager::activateProtocolPlugins()
{
	bool saveList = false;

	foreach (Plugin *plugin, Plugins)
	{
		if (!plugin->isValid() || plugin->info()->type() != "protocol")
			continue;

		if (plugin->shouldBeActivated())
		{
			if (!activatePlugin(plugin))
				saveList = true;
			else // for load-by-default plugins
				plugin->setState(Plugin::PluginStateEnabled);
		}
	}

	// if not all plugins were loaded properly
	// save the list of modules
	if (saveList)
		ConfigurationManager::instance()->flush();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Activate all plugins that are enabled.
 *
 * This method activates all plugins that are either enabled (Plugin::PluginStateEnabled) or new (Plugin::PluginStateNew)
 * with attribute "load by default" set. If given enabled plugin is no longer available replacement plugin is searched
 * (by checking Plugin::replaces()). Any found replacement plugin is activated.
 */
void PluginsManager::activatePlugins()
{
	bool saveList = false;

	foreach (Plugin *plugin, Plugins)
		if (plugin->shouldBeActivated())
		{
			if (!activatePlugin(plugin))
				saveList = true;
			else // for load-by-default plugins
				plugin->setState(Plugin::PluginStateEnabled);
		}

	foreach (Plugin *pluginToReplace, Plugins)
	{
		if (pluginToReplace->isActive() || pluginToReplace->state() != Plugin::PluginStateEnabled)
			continue;

		foreach (Plugin *replacementPlugin, Plugins)
			if (replacementPlugin->state() == Plugin::PluginStateNew && replacementPlugin->isValid() && replacementPlugin->info()->replaces().contains(pluginToReplace->name()))
				if (activatePlugin(replacementPlugin))
				{
					replacementPlugin->setState(Plugin::PluginStateEnabled);
					saveList = true; // list has changed
				}
	}

	// if not all plugins were loaded properly or new plugin was added
	// save the list of modules
	if (saveList)
		ConfigurationManager::instance()->flush();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Activate all plugins that are enabled.
 *
 * This method deactivated all active plugins. First iteration of deactivation check Plugin::usageCounter() value
 * to check if given plugin can be safely removed (no other active plugins depends on it). This procedure is
 * performed for all active plugins until no more plugins can be deactivated. Then second iteration is performed.
 * This time no checks are performed.
 */
void PluginsManager::deactivatePlugins()
{
	ConfigurationManager::instance()->flush();

	foreach (Plugin *plugin, Plugins)
		if (plugin->isActive())
		{
			kdebugm(KDEBUG_INFO, "module: %s, usage: %d\n", qPrintable(plugin->name()), plugin->usageCounter());
		}

	// unloading all not used modules
	// as long as any module were unloaded

	bool deactivated;
	do
	{
		QList<Plugin *> active = activePlugins();
		deactivated = false;
		foreach (Plugin *plugin, active)
			if (plugin->usageCounter() == 0)
				if (deactivatePlugin(plugin, false))
					deactivated = true;
	}
	while (deactivated);

	// we cannot unload more modules in normal way
	// so we are making it brutal ;)
	QList<Plugin *> active = activePlugins();
	foreach (Plugin *plugin, active)
	{
		kdebugm(KDEBUG_PANIC, "WARNING! Could not deactivate module %s, killing\n", qPrintable(plugin->name()));
		deactivatePlugin(plugin, true);
	}

}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Lists all active plugins.
 * @return list of all active plugins
 *
 * This method returns list of all active plugins. Active plugin has its shred library loaded and objects
 * created.
 */
QList<Plugin *> PluginsManager::activePlugins() const
{
	QList<Plugin *> result;
	foreach (Plugin *plugin, Plugins)
		if (plugin->isActive())
			result.append(plugin);
	return result;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Marks all dependencies of given plugin as used.
 * @param plugin plugin which dependencies will be marked as used
 *
 * All depenciec of plugin given as parameter will be marked as used so no accidental deactivation
 * will be possible for them.
 */
void PluginsManager::incDependenciesUsageCount(Plugin *plugin)
{
	if (!plugin->isValid())
		return;

	kdebugmf(KDEBUG_FUNCTION_START, "%s\n", qPrintable(plugin->info()->description()));
	foreach (const QString &pluginName, plugin->info()->dependencies())
	{
		kdebugm(KDEBUG_INFO, "incUsage: %s\n", qPrintable(pluginName));
		usePlugin(pluginName);
	}
	kdebugf2();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Lists all installed plugins names.
 * @return list of all installed plugin names
 *
 * Lists all installed plugins names. Installed plugins are searched in dataDir/kadu/plugins as
 * *.desc files.
 */
QStringList PluginsManager::installedPlugins() const
{
	QDir dir(dataPath("kadu/plugins"), "*.desc");
	dir.setFilter(QDir::Files);

	QStringList installed;
	QStringList entries = dir.entryList();
	foreach (const QString &entry, entries)
		installed.append(entry.left(entry.length() - 5));
	return installed;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Returns name of active plugin that conflicts with given one.
 * @param plugin plugin for which conflict is searched
 * @return name of active plugins that conflicts with given one
 *
 * Return empty string if no active conflict plugin is found or if given plugin is empty or invalid.
 * In other cases name of active conflict plugin is returned. This means:
 * * any active plugin that is in current plugin's PluginInfo::conflicts() list
 * * any active plugin that provides (see PluginInfo::provides()) something that is in current plugin's PluginInfo::conflicts() list
 * * any active plugin that has current plugin it its PluginInfo::conflicts() list
 */
QString PluginsManager::findActiveConflict(Plugin *plugin) const
{
	if (!plugin || !plugin->isValid())
		return QString();

	foreach (const QString &conflict, plugin->info()->conflicts())
	{
		// note that conflict may be something provided, not necessarily a plugin
		QMap<QString, Plugin *>::const_iterator it(Plugins.find(conflict));
		if (it != Plugins.constEnd() && it.value()->isActive())
			return conflict;

		foreach (Plugin *possibleConflict, Plugins)
			if (possibleConflict->isValid() && possibleConflict->isActive())
				foreach (const QString &provided, possibleConflict->info()->provides())
					if (conflict == provided)
						return possibleConflict->name();
	}

	foreach (Plugin *possibleConflict, Plugins)
		if (possibleConflict->isValid() && possibleConflict->isActive())
			foreach (const QString &sit, possibleConflict->info()->conflicts())
				if (sit == plugin->name())
					return plugin->name();

	return QString();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Activates (recursively) all dependencies.
 * @param plugin plugin for which dependencies will be activated
 * @return true if all dependencies were activated
 * @todo remove MessageDialog from this methods, this is not a GUI class
 *
 * Activates all dependencies of plugin and dependencies of these dependencies. If any dependency
 * is not found a message will be displayed to the user and false will be returned. * 
 */
bool PluginsManager::activateDependencies(Plugin *plugin)
{
	if (!plugin || !plugin->isValid())
		return true; // always true

	foreach (const QString &dependencyName, plugin->info()->dependencies())
	{
		if (!Plugins.contains(dependencyName))
		{
			MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("Required module %1 was not found").arg(dependencyName));
			return false;
		}

		if (!activatePlugin(Plugins.value(dependencyName)))
			return false;
	}

	return true;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Returns string with list of all plugins that depends on given one.
 * @param pluginName name of plugin to check dependencies
 * @return string with list of all active plugins that depend on given one
 * @todo ugly, should return QStringList or QList&lt;Plugin *&t;
 *
 * Returns string with list of all active plugins that depend on given one. This string can
 * be displayed to the user.
 */
QString PluginsManager::activeDependentPluginNames(const QString &pluginName) const
{
	QString modules;

	foreach (Plugin *possibleDependentPlugin, Plugins)
		if (possibleDependentPlugin->isValid() && possibleDependentPlugin->isActive())
			if (possibleDependentPlugin->info()->dependencies().contains(pluginName))
				modules += "\n- " + possibleDependentPlugin->name();

	return modules;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Activates given plugin and all its dependencies.
 * @param plugin plugin to activate
 * @return true, if plugin was successfully activated
 * @todo remove message box
 *
 * This method activates given plugin and all its dependencies. Plugin can be activated only when no conflict
 * is found and all dependencies can be activated. In other case false is returned and plugin will not be activated.
 * Please note that no dependency plugin activated in this method will be automatically deactivated if
 * this method fails, so list of active plugins can be changed even if plugin could not be activated.
 *
 * After successfull activation all dependencies are locked using incDependenciesUsageCount() and cannot be
 * deactivated without deactivating plugin. Plugin::usageCounter() of dependencies is increased.
 */
bool PluginsManager::activatePlugin(Plugin *plugin)
{
	if (plugin->isActive())
		return true;

	QString conflict = findActiveConflict(plugin);
	if (!conflict.isEmpty())
	{
		MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("Module %1 conflicts with: %2").arg(plugin->name(), conflict));
		return false;
	}

	if (!activateDependencies(plugin))
		return false;

	bool result = plugin->activate();
	if (result)
		incDependenciesUsageCount(plugin);

	return result;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Deactivates given plugin.
 * @param plugin plugin to deactivate
 * @param force if true, no check for usage will be performed
 * @return true, if plugin was successfully deactivated
 * @todo remove message box
 *
 * This method deactivates given plugin. Deactivation can be performed only when plugin is no longed in use (its
 * Plugin::usageCounter() returns 0) or when force parameter is set to true.
 *
 * After successfull deactivation all dependenecies are released - their Plugin::usageCounter() is decreaced.
 */
bool PluginsManager::deactivatePlugin(Plugin *plugin, bool force)
{
	kdebugmf(KDEBUG_FUNCTION_START, "name:'%s' force:%d usage: %d\n", qPrintable(plugin->name()), force, plugin->usageCounter());

	if (plugin->usageCounter() > 0 && !force)
	{
		MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("Module %1 cannot be deactivated because it is being used by the following modules:%2").arg(plugin->name()).arg(activeDependentPluginNames(plugin->name())));
		kdebugf2();
		return false;
	}

	foreach (const QString &i, plugin->info()->dependencies())
		releasePlugin(i);

	plugin->deactivate();
	return true;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Increases usage counter for given plugin
 * @param pluginName name of plugin to increase usage pointer
 *
 * This method increases usage counter for given plugin. Use it from plugin code when opening
 * dialogs or performing long operatrions to prevent unloading plugins. It is used also to prevent
 * unloading plugins when other loaded plugins are dependent on them.
 *
 * After plugin is no longer needes (operation has finished, dialog is closed) releasePlugin() must
 * be called with the same parameter.
 */
void PluginsManager::usePlugin(const QString &pluginName)
{
	if (Plugins.contains(pluginName))
		Plugins.value(pluginName)->incUsage();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Decreases usage counter for given plugin
 * @param pluginName name of plugin to decrease usage pointer
 *
 * This method decreases usage counter for given plugin. Use it from plugin after calling usePlugin()
 * when locking plugin is no longer needed (for example when long operation has finished or a dialog
 * was closed).
 *
 * It can be only called after usePlugin() with the same parameter.
 */
void PluginsManager::releasePlugin(const QString &pluginName)
{
	if (Plugins.contains(pluginName))
		Plugins.value(pluginName)->decUsage();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Shows plugins manager window
 * @todo remove
 */
void PluginsManager::showWindow(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	kdebugf();

	if (!Window)
	{
		Window = new ModulesWindow();
		connect(Window, SIGNAL(destroyed()), this, SLOT(dialogDestroyed()));
		Window->show();
	}

	_activateWindow(Window);

	kdebugf2();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Called after plugins manager window got closed
 * @todo remove
 */
void PluginsManager::dialogDestroyed()
{
	kdebugf();
	Window = 0;
	kdebugf2();
}
