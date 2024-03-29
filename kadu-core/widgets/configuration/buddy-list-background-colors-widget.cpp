/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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
#include <QtWidgets/QLabel>

#include "configuration/config-file-data-manager.h"
#include "widgets/color-button.h"
#include "widgets/configuration/config-group-box.h"
#include "widgets/configuration/configuration-widget.h"
#include "windows/main-configuration-window.h"

#include "buddy-list-background-colors-widget.h"
#include "moc_buddy-list-background-colors-widget.cpp"

BuddyListBackgroundColorsWidget::BuddyListBackgroundColorsWidget(MainConfigurationWindow *mainWindow)
        : m_mainWindow{mainWindow}
{
    createGui();
    loadConfiguration();

    connect(m_mainWindow, SIGNAL(configurationWindowApplied()), this, SLOT(configurationApplied()));
}

void BuddyListBackgroundColorsWidget::createGui()
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    colorButton = new ColorButton(this);
    alternateColorButton = new ColorButton(this);

    layout->addWidget(colorButton);
    layout->addWidget(alternateColorButton);

    ConfigGroupBox *groupBox = m_mainWindow->widget()->configGroupBox("Look", "Buddies list", "Background");

    groupBox->addWidgets(new QLabel(QCoreApplication::translate("@default", "Background colors") + ':', this), this);
}

void BuddyListBackgroundColorsWidget::loadConfiguration()
{
    if (!m_mainWindow || m_mainWindow->dataManager())
        return;

    colorButton->setColor(m_mainWindow->dataManager()->readEntry("Look", "UserboxBgColor").value<QColor>());
    alternateColorButton->setColor(
        m_mainWindow->dataManager()->readEntry("Look", "UserboxAlternateBgColor").value<QColor>());
}

void BuddyListBackgroundColorsWidget::configurationApplied()
{
    if (!m_mainWindow || m_mainWindow->dataManager())
        return;

    m_mainWindow->dataManager()->writeEntry("Look", "UserboxBgColor", QVariant(colorButton->color().name()));
    m_mainWindow->dataManager()->writeEntry(
        "Look", "UserboxAlternateBgColor", QVariant(alternateColorButton->color().name()));
}
