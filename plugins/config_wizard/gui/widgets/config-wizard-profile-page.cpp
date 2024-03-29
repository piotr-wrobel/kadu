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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "core/myself.h"
#include "languages-manager.h"

#include "config-wizard-profile-page.h"
#include "moc_config-wizard-profile-page.cpp"

ConfigWizardProfilePage::ConfigWizardProfilePage(QWidget *parent) : ConfigWizardPage(parent)
{
}

ConfigWizardProfilePage::~ConfigWizardProfilePage()
{
}

void ConfigWizardProfilePage::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void ConfigWizardProfilePage::setLanguagesManager(LanguagesManager *languagesManager)
{
    m_languagesManager = languagesManager;
}

void ConfigWizardProfilePage::setMyself(Myself *myself)
{
    m_myself = myself;
}

void ConfigWizardProfilePage::init()
{
    setDescription(
        tr("<h3>Welcome to Kadu Instant Messenger</h3>"
           "<p>This wizard will help you to configure the basic settings of Kadu.</p>"
           "<p>Please choose a preferred language and create a nickname</p>"));

    createGui();
}

void ConfigWizardProfilePage::createGui()
{
    formLayout()->addRow(new QLabel(tr("<h3>Profile setup</h3>"), this));

    LanguagesCombo = new QComboBox(this);
    setLanguages();
    formLayout()->addRow(tr("Language") + ':', LanguagesCombo);

    QLabel *restartInfo = new QLabel(
        "<font size='-1'><i>" +
            (QCoreApplication::translate(
                "@default",
                // NOTE: it's the same string as in varia/configuration/dialog.ui
                "Kadu needs to be restarted before changes to the language settings will take effect.")) +
            "</i></font>",
        this);
    formLayout()->addRow(QString(), restartInfo);

    NickNameEdit = new QLineEdit(this);
    NickNameEdit->setMaximumWidth(300);
    formLayout()->addRow(tr("Nickname") + ':', NickNameEdit);
}

void ConfigWizardProfilePage::setLanguages()
{
    for (QMap<QString, QString>::const_iterator it = m_languagesManager->languages().constBegin(),
                                                end = m_languagesManager->languages().constEnd();
         it != end; ++it)
        LanguagesCombo->addItem(it.value(), it.key());
}

void ConfigWizardProfilePage::initializePage()
{
    int languageIndex = LanguagesCombo->findData(m_configuration->deprecatedApi()->readEntry("General", "Language"));
    if (-1 == languageIndex)
        languageIndex = LanguagesCombo->findData("en");
    if (-1 != languageIndex)
        LanguagesCombo->setCurrentIndex(languageIndex);

    NickNameEdit->setText(m_configuration->deprecatedApi()->readEntry("General", "Nick", "Me"));
}

void ConfigWizardProfilePage::acceptPage()
{
    m_configuration->deprecatedApi()->writeEntry(
        "General", "Language", LanguagesCombo->itemData(LanguagesCombo->currentIndex()).toString());
    m_configuration->deprecatedApi()->writeEntry("General", "Nick", NickNameEdit->text());

    // TODO: check if needed
    m_myself->buddy().setDisplay(NickNameEdit->text());
}
