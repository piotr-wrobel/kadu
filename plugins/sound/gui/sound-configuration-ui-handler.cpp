/*
 * %kadu copyright begin%
 * Copyright 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "sound-configuration-ui-handler.h"
#include "moc_sound-configuration-ui-handler.cpp"

#include "gui/sound-configuration-widget.h"
#include "sound-manager.h"
#include "sound-theme-manager.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "misc/paths-provider.h"
#include "plugin/plugin-injected-factory.h"
#include "themes.h"
#include "widgets/configuration/config-combo-box.h"
#include "widgets/configuration/configuration-widget.h"
#include "widgets/path-list-edit.h"
#include "windows/main-configuration-window.h"

SoundConfigurationUiHandler::SoundConfigurationUiHandler(QObject *parent) : QObject{parent}
{
}

SoundConfigurationUiHandler::~SoundConfigurationUiHandler()
{
}

void SoundConfigurationUiHandler::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void SoundConfigurationUiHandler::setSoundManager(SoundManager *soundManager)
{
    m_soundManager = soundManager;
}

void SoundConfigurationUiHandler::setSoundThemeManager(SoundThemeManager *soundThemeManager)
{
    m_soundThemeManager = soundThemeManager;
}

void SoundConfigurationUiHandler::setSoundThemes()
{
    if (!m_themesComboBox)
        return;

    m_soundThemeManager->themes()->setPaths(m_themesPaths->pathList());

    auto soundThemeNames = m_soundThemeManager->themes()->themes();
    soundThemeNames.sort();

    auto soundThemeValues = soundThemeNames;

    soundThemeNames.prepend(tr("Custom"));
    soundThemeValues.prepend("Custom");

    m_themesComboBox->setItems(soundThemeValues, soundThemeNames);
    m_themesComboBox->setCurrentIndex(m_themesComboBox->findText(m_soundThemeManager->themes()->theme()));
}

void SoundConfigurationUiHandler::connectWidgets()
{
    if (!m_themesComboBox || !m_configurationWidget)
        return;

    connect(m_themesComboBox, SIGNAL(activated(int)), m_configurationWidget, SLOT(themeChanged(int)));
    connect(m_themesComboBox, SIGNAL(activated(const QString &)), this, SLOT(themeChanged(const QString &)));
    m_configurationWidget->themeChanged(m_themesComboBox->currentIndex());
}

void SoundConfigurationUiHandler::mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow)
{
    connect(
        mainConfigurationWindow->widget()->widgetById("sound/testPlay"), SIGNAL(clicked()), m_soundManager,
        SLOT(testSoundPlaying()));

    m_themesComboBox = static_cast<ConfigComboBox *>(mainConfigurationWindow->widget()->widgetById("sound/themes"));
    m_themesPaths = static_cast<PathListEdit *>(mainConfigurationWindow->widget()->widgetById("soundPaths"));
    // connect(ThemesPaths, SIGNAL(changed()), SoundManager::instance(), SLOT(setSoundThemes()));

    setSoundThemes();

    connectWidgets();
}

void SoundConfigurationUiHandler::mainConfigurationWindowDestroyed()
{
    m_themesComboBox = nullptr;
    m_themesPaths = nullptr;
}

void SoundConfigurationUiHandler::mainConfigurationWindowApplied()
{
    if (m_themesComboBox->currentIndex() != 0)
        m_soundThemeManager->applyTheme(m_themesComboBox->currentText());

    m_configurationWidget->themeChanged(m_themesComboBox->currentIndex());
}

NotifierConfigurationWidget *SoundConfigurationUiHandler::createConfigurationWidget(QWidget *parent)
{
    m_configurationWidget = m_pluginInjectedFactory->makeInjected<SoundConfigurationWidget>(m_soundManager, parent);
    connect(m_configurationWidget, SIGNAL(soundFileEdited()), this, SLOT(soundFileEdited()));

    connectWidgets();

    return m_configurationWidget;
}

void SoundConfigurationUiHandler::themeChanged(const QString &theme)
{
    m_soundThemeManager->applyTheme(theme);
}

void SoundConfigurationUiHandler::soundFileEdited()
{
    if (m_themesComboBox->currentIndex() != 0)
        m_themesComboBox->setCurrentIndex(0);
}
