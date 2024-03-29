/*
 * %kadu copyright begin%
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTextBrowser>

#include "config-wizard-page.h"
#include "moc_config-wizard-page.cpp"

ConfigWizardPage::ConfigWizardPage(QWidget *parent) : QWizardPage(parent)
{
    createGui();
}

ConfigWizardPage::~ConfigWizardPage()
{
}

void ConfigWizardPage::createGui()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(5);

    DescriptionPane = new QTextBrowser(this);
    DescriptionPane->setFocusPolicy(Qt::ClickFocus);
    mainLayout->addWidget(DescriptionPane, 2);

    QWidget *formWidget = new QWidget(this);
    FormLayout = new QFormLayout(formWidget);

    mainLayout->addWidget(formWidget, 5);
}

void ConfigWizardPage::acceptPage()
{
}

void ConfigWizardPage::rejectPage()
{
}

void ConfigWizardPage::setDescription(const QString &description)
{
    DescriptionPane->setText(description);
}
