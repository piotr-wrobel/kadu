/*
 * %kadu copyright begin%
 * Copyright 2009, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2008, 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>

#include "config-check-box.h"
#include "moc_config-check-box.cpp"

#include "widgets/configuration/config-group-box.h"

ConfigCheckBox::ConfigCheckBox(
    const QString &section, const QString &item, const QString &widgetCaption, const QString &toolTip,
    ConfigGroupBox *parentConfigGroupBox, ConfigurationWindowDataManager *dataManager)
        : QCheckBox(widgetCaption, parentConfigGroupBox->widget()),
          ConfigWidgetValue(section, item, widgetCaption, toolTip, parentConfigGroupBox, dataManager)
{
    createWidgets();
}

ConfigCheckBox::ConfigCheckBox(ConfigGroupBox *parentConfigGroupBox, ConfigurationWindowDataManager *dataManager)
        : QCheckBox(parentConfigGroupBox->widget()), ConfigWidgetValue(parentConfigGroupBox, dataManager)
{
}

void ConfigCheckBox::createWidgets()
{
    setText(QCoreApplication::translate("@default", widgetCaption.toUtf8().constData()));
    parentConfigGroupBox->addWidget(this, true);

    if (!ConfigWidget::toolTip.isEmpty())
        setToolTip(QCoreApplication::translate("@default", ConfigWidget::toolTip.toUtf8().constData()));
}

void ConfigCheckBox::loadConfiguration()
{
    if (!dataManager)
        return;
    setChecked(dataManager->readEntry(section, item).toBool());
    emit toggled(isChecked());
}

void ConfigCheckBox::saveConfiguration()
{
    if (!dataManager)
        return;
    dataManager->writeEntry(section, item, QVariant(isChecked() ? "true" : "false"));
}
