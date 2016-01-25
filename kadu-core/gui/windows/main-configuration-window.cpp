/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2011 Przemysław Rudy (prudy1@o2.pl)
 * Copyright 2010, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2010, 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Maciej Płaza (plaza.maciej@gmail.com)
 * Copyright 2010 badboy (badboy@gen2.org)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStyleFactory>

#include "configuration/config-file-data-manager.h"
#include "configuration/gui/configuration-ui-handler.h"

#include "accounts/account-manager.h"
#include "accounts/account.h"
#include "buddies/buddy.h"
#include "compression/archive-extractor.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "configuration/gui/configuration-ui-handler.h"
#include "configuration/gui/configuration-ui-handler-repository.h"
#include "contacts/contact.h"
#include "core/injected-factory.h"
#include "gui/widgets/buddy-info-panel.h"
#include "gui/widgets/configuration/buddy-list-background-colors-widget.h"
#include "gui/widgets/configuration/config-check-box.h"
#include "gui/widgets/configuration/config-combo-box.h"
#include "gui/widgets/configuration/config-group-box.h"
#include "gui/widgets/configuration/config-line-edit.h"
#include "gui/widgets/configuration/config-list-widget.h"
#include "gui/widgets/configuration/config-path-list-edit.h"
#include "gui/widgets/configuration/config-preview.h"
#include "gui/widgets/configuration/config-syntax-editor.h"
#include "gui/widgets/configuration/configuration-widget.h"
#include "gui/widgets/proxy-combo-box.h"
#include "gui/widgets/tool-tip-class-manager.h"
#include "gui/windows/kadu-window-service.h"
#include "gui/windows/kadu-window.h"
#include "gui/windows/message-dialog.h"
#include "misc/paths-provider.h"
#include "network/proxy/network-proxy.h"
#include "plugin/gui/plugin-list/plugin-list-widget.h"
#include "status/status-container.h"
#include "status/status.h"
#include "themes/icon-theme-manager.h"

#include "debug.h"
#include "languages-manager.h"

#include "main-configuration-window.h"

#if defined(Q_OS_UNIX)
#include "os/x11/x11tools.h" // this should be included as last one,
#undef KeyPress
#undef Status            // and Status defined by Xlib.h must be undefined
#endif

const char *MainConfigurationWindow::SyntaxText = QT_TRANSLATE_NOOP
(
	"@default", "Syntax: %s - status, %d - description, %i - ip, %n - nick, %a - altnick, %f - first name\n"
	"%r - surname, %m - mobile, %u - uin, %g - group\n"
	"%h - gg version, %v - revDNS, %p - port, %e - email, %x - max image size, %z - gender (0/1/2)\n"
);

const char *MainConfigurationWindow::SyntaxTextNotify = QT_TRANSLATE_NOOP
(
	"@default", "Syntax: %s - status, %d - description, %i - ip, %n - nick, %a - altnick, %f - first name\n"
	"%r - surname, %m - mobile, %u - uin, %g - group\n"
	"%h - gg version, %v - revDNS, %p - port, %e - email, %x - max image size, %z - gender (0/1/2),\n"
	"#{protocol} - protocol that triggered event,\n"
	"#{event} - name of event,\n"
);

void MainConfigurationWindow::configurationUiHandlerAdded(ConfigurationUiHandler *configurationUiHandler)
{
	configurationUiHandler->mainConfigurationWindowCreated(this);
}

void MainConfigurationWindow::configurationUiHandlerRemoved(ConfigurationUiHandler *configurationUiHandler)
{
	Q_UNUSED(configurationUiHandler)
}

MainConfigurationWindow::MainConfigurationWindow(QWidget *parent) :
		ConfigurationWindow("MainConfiguration", tr("Kadu configuration"), "General", new ConfigFileDataManager(), parent)
{
}

MainConfigurationWindow::~MainConfigurationWindow()
{
	for (auto configurationUiHandler : m_configurationUiHandlerRepository)
		configurationUiHandler->mainConfigurationWindowDestroyed();
}

void MainConfigurationWindow::setAccountManager(AccountManager *accountManager)
{
	m_accountManager = accountManager;
}

void MainConfigurationWindow::setConfigurationUiHandlerRepository(ConfigurationUiHandlerRepository *configurationUiHandlerRepository)
{
	m_configurationUiHandlerRepository = configurationUiHandlerRepository;
}

void MainConfigurationWindow::setIconThemeManager(IconThemeManager *iconThemeManager)
{
	m_iconThemeManager = iconThemeManager;
}

void MainConfigurationWindow::setKaduWindowService(KaduWindowService *kaduWindowService)
{
	m_kaduWindowService = kaduWindowService;
}

void MainConfigurationWindow::setLanguagesManager(LanguagesManager *languagesManager)
{
	m_languagesManager = languagesManager;
}

void MainConfigurationWindow::setPathsProvider(PathsProvider *pathsProvider)
{
	m_pathsProvider = pathsProvider;
}

void MainConfigurationWindow::setToolTipClassManager(ToolTipClassManager *toolTipClassManager)
{
	m_toolTipClassManager = toolTipClassManager;
}

void MainConfigurationWindow::init()
{
	setWindowRole("kadu-configuration");

	widget()->appendUiFile(m_pathsProvider->dataPath() + QLatin1String("configuration/dialog.ui"));

#ifndef DEBUG_ENABLED
	widget()->widgetById("debug")->hide();
#endif

#ifndef Q_OS_WIN
	widget()->widgetById("startup")->hide();
	widget()->widgetById("hideMainWindowFromTaskbar")->hide();
#endif

#if !defined(Q_OS_UNIX)
	widget()->widgetById("windowActivationMethod")->hide();
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_WIN)
	widget()->widgetById("notify/fullscreenSilentMode")->hide();
#endif

#if !defined(Q_OS_UNIX)
	widget()->widgetById("useTransparency")->hide();
	widget()->widgetById("userboxTransparency")->hide();
	widget()->widgetById("userboxAlpha")->hide();
	widget()->widgetById("userboxBlur")->hide();
#endif

	onStartupSetLastDescription = static_cast<QCheckBox *>(widget()->widgetById("onStartupSetLastDescription"));
	disconnectDescription = static_cast<QLineEdit *>(widget()->widgetById("disconnectDescription"));
	onStartupSetDescription = static_cast<QLineEdit *>(widget()->widgetById("onStartupSetDescription"));

	Account account = m_accountManager->defaultAccount();
	if (!account.isNull() && account.protocolHandler())
	{
		disconnectDescription->setMaxLength(account.statusContainer()->maxDescriptionLength());
		onStartupSetDescription->setMaxLength(account.statusContainer()->maxDescriptionLength());
	}

	connect(widget()->widgetById("disconnectWithCurrentDescription"), SIGNAL(activated(int)), this, SLOT(onChangeShutdownStatus(int)));
	connect(onStartupSetLastDescription, SIGNAL(activated(int)), this, SLOT(onChangeStartupDescription(int)));

	connect(widget()->widgetById("startupStatus"), SIGNAL(activated(int)), this, SLOT(onChangeStartupStatus(int)));
	connect(widget()->widgetById("lookChatAdvanced"), SIGNAL(clicked()), this, SLOT(showLookChatAdvanced()));
	connect(widget()->widgetById("installIconTheme"), SIGNAL(clicked()), this, SLOT(installIconTheme()));

	Preview *infoPanelSyntaxPreview = static_cast<Preview *>(widget()->widgetById("infoPanelSyntaxPreview"));
	connect(infoPanelSyntaxPreview, SIGNAL(needFixup(QString &)), m_kaduWindowService->kaduWindow()->infoPanel(), SLOT(styleFixup(QString &)));
	connect(widget()->widgetById("infoPanelSyntax"), SIGNAL(syntaxChanged(const QString &)), infoPanelSyntaxPreview, SLOT(syntaxChanged(const QString &)));

	widget()->widgetById("parseStatus")->setToolTip(QCoreApplication::translate("@default", SyntaxText));
	(static_cast<ConfigSyntaxEditor *>(widget()->widgetById("infoPanelSyntax")))->setSyntaxHint(QCoreApplication::translate("@default", SyntaxText));

	userboxTransparency = static_cast<QCheckBox *>(widget()->widgetById("userboxTransparency"));
	userboxAlpha = static_cast<QSlider *>(widget()->widgetById("userboxAlpha"));
	userboxBlur = static_cast<QCheckBox *>(widget()->widgetById("userboxBlur"));

	buddyColors = new BuddyListBackgroundColorsWidget(this);

	PluginList = injectedFactory()->makeInjected<PluginListWidget>(this);
	PluginList->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

	connect(this, SIGNAL(configurationWindowApplied()), this, SLOT(applied()));
	connect(m_configurationUiHandlerRepository, SIGNAL(configurationUiHandlerAdded(ConfigurationUiHandler*)),
	        this, SLOT(configurationUiHandlerAdded(ConfigurationUiHandler*)));
	connect(m_configurationUiHandlerRepository, SIGNAL(configurationUiHandlerRemoved(ConfigurationUiHandler*)),
	        this, SLOT(configurationUiHandlerRemoved(ConfigurationUiHandler*)));

	triggerCompositingStateChanged();
}

void MainConfigurationWindow::applied()
{
	for (auto configurationUiHandler : m_configurationUiHandlerRepository)
		configurationUiHandler->mainConfigurationWindowApplied();
}

void MainConfigurationWindow::compositingEnabled()
{
	auto userboxTransparencyGroup = widget()->widgetById("userboxTransparencyGroup")->parentWidget();
	userboxTransparencyGroup->setVisible(true);
	userboxTransparency->setEnabled(true);
	userboxTransparency->blockSignals(false);
	userboxAlpha->setEnabled(userboxTransparency->isChecked());
	userboxBlur->setEnabled(userboxTransparency->isChecked());
}

void MainConfigurationWindow::compositingDisabled()
{
	auto userboxTransparencyGroup = widget()->widgetById("userboxTransparencyGroup")->parentWidget();
	userboxTransparencyGroup->setVisible(false);
	userboxTransparency->setEnabled(false);
	userboxTransparency->blockSignals(true);
	userboxAlpha->setEnabled(false);
	userboxBlur->setEnabled(false);
}

void MainConfigurationWindow::show()
{
	if (!isVisible())
	{
		setLanguages();
		setIconThemes();
		setToolTipClasses();
	}

	ConfigurationWindow::show();
}

void MainConfigurationWindow::onChangeStartupStatus(int index)
{
	onStartupSetLastDescription->setEnabled(index != 6);
	widget()->widgetById("startupStatusInvisibleWhenLastWasOffline")->setEnabled(index == 0);
	widget()->widgetById("onStartupSetDescription")->setEnabled(!onStartupSetLastDescription->isChecked() && index != 6);
}

void MainConfigurationWindow::onChangeStartupDescription(int index)
{
	onStartupSetDescription->setEnabled(index == 1);
}

void MainConfigurationWindow::onChangeShutdownStatus(int index)
{
	disconnectDescription->setEnabled(index == 1);
}

void MainConfigurationWindow::setLanguages()
{
	ConfigComboBox *languages = static_cast<ConfigComboBox *>(widget()->widgetById("languages"));

	languages->setItems(m_languagesManager->languages().keys(), m_languagesManager->languages().values());
}

void MainConfigurationWindow::installIconTheme()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open icon theme archive"), QDir::home().path(), tr("Archive (*.tar.xz *.tar.gz *.tar.bz2 *.tar *.zip)"));

	if (fileName.isEmpty())
		return;

	const QString &profilePath = m_pathsProvider->profilePath();
	ArchiveExtractor extractor;
	bool success = extractor.extract(fileName, profilePath + "icons");
	if (success)
	{
		setIconThemes();
	}
	else
	{
		MessageDialog::show(KaduIcon("dialog-warning"), tr("Installation failed"), tr(extractor.message().toLocal8Bit().data()), QMessageBox::Ok, widget());
	}
}

void MainConfigurationWindow::setIconThemes()
{
	ConfigListWidget *iconThemes = static_cast<ConfigListWidget *>(widget()->widgetById("iconThemes"));
	iconThemes->clear();

	m_iconThemeManager->loadThemes();

	(void)QT_TRANSLATE_NOOP("@default", "default");

	QStringList values;
	QStringList captions;
	foreach (const Theme &theme, m_iconThemeManager->themes())
	{
		values.append(theme.name());
		captions.append(QCoreApplication::translate("@default", theme.name().toUtf8().constData()));
	}

	iconThemes->setItems(values, captions);
	iconThemes->setCurrentItem(m_iconThemeManager->currentTheme().name());

	QStringList iconPaths;
	iconPaths
			<< "protocols/xmpp/online"
			<< "protocols/gadu-gadu/online"
			<< "protocols/common/message"
			<< "preferences-other";

	QList<QIcon> icons;
	foreach (const Theme &theme, m_iconThemeManager->themes())
	{
		QPixmap combinedIcon(iconPaths.count() * 36, 36);
		combinedIcon.fill(Qt::transparent);

		QPainter iconPainter(&combinedIcon);

		for (int i = 0; i < iconPaths.count(); i++)
		{
			KaduIcon kaduIcon(iconPaths.at(i));
			kaduIcon.setThemePath(theme.path());
			QIcon icon = kaduIcon.icon();
			icon.paint(&iconPainter, 2 + 36 * i, 2, 32, 32);
		}

		icons.append(QIcon(combinedIcon));
	}

	iconThemes->setIconSize(QSize(iconPaths.count() * 36, 36));
	iconThemes->setIcons(icons);
}

void MainConfigurationWindow::setToolTipClasses()
{
	QStringList captions;
	QStringList values;
	captions << tr("None");
	values << QLatin1String("none");

	QStringList toolTipClasses = m_toolTipClassManager->getToolTipClasses();
	foreach(const QString &toolTipClass, toolTipClasses)
	{
		captions << QCoreApplication::translate("@default", toolTipClass.toUtf8().constData());
		values << toolTipClass;
	}

	static_cast<ConfigComboBox *>(widget()->widgetById("toolTipClasses"))->setItems(values, captions);
}

void MainConfigurationWindow::showLookChatAdvanced()
{
	if (!lookChatAdvanced)
	{
		lookChatAdvanced = new ConfigurationWindow("LookChatAdvanced", tr("Advanced chat's look configuration"), "General", dataManager());
		lookChatAdvanced.data()->widget()->appendUiFile(m_pathsProvider->dataPath() + QLatin1String("configuration/dialog-look-chat-advanced.ui"));
	}

	lookChatAdvanced.data()->show();
}

#include "moc_main-configuration-window.cpp"
