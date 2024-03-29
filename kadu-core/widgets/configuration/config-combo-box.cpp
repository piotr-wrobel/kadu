/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2010, 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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
#include <QtXml/QDomElement>

#include "config-combo-box.h"
#include "moc_config-combo-box.cpp"

#include "widgets/configuration/config-group-box.h"

ConfigComboBox::ConfigComboBox(
    const QString &section, const QString &item, const QString &widgetCaption, const QString &toolTip,
    const QStringList &itemValues, const QStringList &itemCaptions, ConfigGroupBox *parentConfigGroupBox,
    ConfigurationWindowDataManager *dataManager)
        : QComboBox(parentConfigGroupBox->widget()),
          ConfigWidgetValue(section, item, widgetCaption, toolTip, parentConfigGroupBox, dataManager), label(0),
          saveIndexNotCaption(0)
{
    Q_UNUSED(itemValues)
    Q_UNUSED(itemCaptions)

    createWidgets();
}

ConfigComboBox::ConfigComboBox(ConfigGroupBox *parentConfigGroupBox, ConfigurationWindowDataManager *dataManager)
        : QComboBox(parentConfigGroupBox->widget()), ConfigWidgetValue(parentConfigGroupBox, dataManager), label(0),
          saveIndexNotCaption(0)
{
}

ConfigComboBox::~ConfigComboBox()
{
    if (label)
        delete label;
}

void ConfigComboBox::setItems(const QStringList &itemValues, const QStringList &itemCaptions)
{
    this->itemValues = itemValues;
    this->itemCaptions = itemCaptions;

    clear();
    insertItems(0, itemCaptions);
}

void ConfigComboBox::setCurrentItem(const QString &value)
{
    setCurrentIndex(itemValues.indexOf(value));
}

QString ConfigComboBox::currentItemValue()
{
    int index = currentIndex();

    if ((index < 0) || (index >= itemValues.size()))
        return QString();

    return itemValues.at(index);
}

void ConfigComboBox::createWidgets()
{
    label = new QLabel(
        QCoreApplication::translate("@default", widgetCaption.toUtf8().constData()) + ':',
        parentConfigGroupBox->widget());
    parentConfigGroupBox->addWidgets(label, this);

    clear();
    insertItems(0, itemCaptions);

    if (!ConfigWidget::toolTip.isEmpty())
    {
        setToolTip(QCoreApplication::translate("@default", ConfigWidget::toolTip.toUtf8().constData()));
        label->setToolTip(QCoreApplication::translate("@default", ConfigWidget::toolTip.toUtf8().constData()));
    }
}

void ConfigComboBox::loadConfiguration()
{
    if (!dataManager)
        return;

    if (saveIndexNotCaption)
        setCurrentIndex(dataManager->readEntry(section, item).toInt());
    else
        setCurrentIndex(itemValues.indexOf(dataManager->readEntry(section, item).toString()));

    emit activated(currentIndex());
}

void ConfigComboBox::saveConfiguration()
{
    if (!dataManager)
        return;

    int index = currentIndex();

    if ((index < 0) || (index >= itemValues.size()))
        return;

    if (saveIndexNotCaption)
        dataManager->writeEntry(section, item, currentIndex());
    else
        dataManager->writeEntry(section, item, QVariant(itemValues[currentIndex()]));
}

void ConfigComboBox::setVisible(bool visible)
{
    label->setVisible(visible);
    QComboBox::setVisible(visible);
}

bool ConfigComboBox::fromDomElement(QDomElement domElement)
{
    saveIndexNotCaption = QVariant(domElement.attribute("save-index", "false")).toBool();

    QDomNodeList children = domElement.childNodes();
    uint length = children.length();
    for (uint i = 0; i < length; i++)
    {
        QDomNode node = children.item(static_cast<int>(i));
        if (node.isElement())
        {
            QDomElement element = node.toElement();
            if (element.tagName() != "item")
                continue;

            itemValues.append(element.attribute("value"));
            itemCaptions.append(
                QCoreApplication::translate("@default", element.attribute("caption").toUtf8().constData()));

            addItem(QCoreApplication::translate("@default", element.attribute("caption").toUtf8().constData()));
        }
    }

    return ConfigWidgetValue::fromDomElement(domElement);
}
