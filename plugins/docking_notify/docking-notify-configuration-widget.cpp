/*
 * %kadu copyright begin%
 * Copyright 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>

#include "activate.h"
#include "configuration/configuration.h"
#include "misc/paths-provider.h"
#include "notification/notifier-configuration-data-manager.h"
#include "plugin/plugin-injected-factory.h"
#include "widgets/configuration/config-combo-box.h"
#include "widgets/configuration/configuration-widget.h"
#include "widgets/configuration/notify-group-box.h"
#include "windows/configuration-window.h"
#include "windows/main-configuration-window.h"

#include "docking-notify-configuration-widget.h"
#include "moc_docking-notify-configuration-widget.cpp"

DockingNotifyConfigurationWidget::DockingNotifyConfigurationWidget(QWidget *parent)
        : NotifierConfigurationWidget(parent)
{
    QPushButton *configureButton = new QPushButton(tr("Configure"));
    connect(configureButton, SIGNAL(clicked()), this, SLOT(showConfigurationWindow()));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(configureButton);

    static_cast<NotifyGroupBox *>(parent)->addWidget(this);
}

void DockingNotifyConfigurationWidget::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void DockingNotifyConfigurationWidget::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void DockingNotifyConfigurationWidget::setPathsProvider(PathsProvider *pathsProvider)
{
    m_pathsProvider = pathsProvider;
}

void DockingNotifyConfigurationWidget::saveNotifyConfigurations()
{
}

void DockingNotifyConfigurationWidget::switchToEvent(const QString &event)
{
    currentNotificationEvent = event;
}

void DockingNotifyConfigurationWidget::showConfigurationWindow()
{
    NotifierConfigurationDataManager *dataManager =
        NotifierConfigurationDataManager::dataManagerForEvent(m_pluginInjectedFactory, currentNotificationEvent);
    ConfigurationWindow *configWindow = m_pluginInjectedFactory->makeInjected<ConfigurationWindow>(
        "Qt4DockingNotificationEventConfiguration", tr("Tray icon balloon's look configuration"), "Qt4DockingNotify",
        dataManager);

    dataManager->configurationWindowCreated(configWindow);

    configWindow->widget()->appendUiFile(
        m_pathsProvider->dataPath() + QStringLiteral("plugins/configuration/docking-notify.ui"));

    QString tooltip = QCoreApplication::translate("@default", MainConfigurationWindow::SyntaxTextNotify) +
                      tr("\n%&t - title (eg. New message) %&m - notification text (eg. Message from Jim), %&d - "
                         "details (eg. message quotation),\n%&i - notification icon");

    configWindow->widget()->widgetById("Title")->setToolTip(tooltip);
    configWindow->widget()->widgetById("Syntax")->setToolTip(tooltip);

    configWindow->show();
    _activateWindow(m_configuration, configWindow);
}
