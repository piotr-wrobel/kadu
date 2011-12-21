/*
 * %kadu copyright begin%
 * Copyright 2009, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2008, 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtXml/QDomElement>

#include "configuration/configuration-window-data-manager.h"
#include "gui/widgets/configuration/config-group-box.h"
#include "gui/widgets/configuration/config-list-widget.h"

#include "debug.h"

ConfigListWidget::ConfigListWidget(const QString &widgetCaption, const QString &toolTip,
		const QStringList &itemValues, const QStringList &itemCaptions, ConfigGroupBox *parentConfigGroupBox, ConfigurationWindowDataManager *dataManager)
	: QListWidget(parentConfigGroupBox->widget()), ConfigWidget(widgetCaption, toolTip, parentConfigGroupBox, dataManager), label(0)
{
	Q_UNUSED(itemValues)
	Q_UNUSED(itemCaptions)

	createWidgets();
}

ConfigListWidget::ConfigListWidget(ConfigGroupBox *parentConfigGroupBox, ConfigurationWindowDataManager *dataManager)
	: QListWidget(parentConfigGroupBox->widget()), ConfigWidget(parentConfigGroupBox, dataManager), label(0)
{
}

ConfigListWidget::~ConfigListWidget()
{
	if (label)
		delete label;
}

void ConfigListWidget::setItems(const QStringList &itemValues, const QStringList &itemCaptions)
{
	this->itemValues = itemValues;
	this->itemCaptions = itemCaptions;

	clear();
	addItems(itemCaptions);
}

void ConfigListWidget::createWidgets()
{
	kdebugf();

	label = new QLabel(qApp->translate("@default", widgetCaption.toUtf8().constData()) + ':', parentConfigGroupBox->widget());
	parentConfigGroupBox->addWidgets(label, this);

	clear();
	addItems(itemCaptions);

	if (!ConfigWidget::toolTip.isEmpty())
	{
		setToolTip(qApp->translate("@default", ConfigWidget::toolTip.toUtf8().constData()));
		label->setToolTip(qApp->translate("@default", ConfigWidget::toolTip.toUtf8().constData()));
	}
}

void ConfigListWidget::setVisible(bool visible)
{
	label->setVisible(visible);
	QListWidget::setVisible(visible);
}

bool ConfigListWidget::fromDomElement(QDomElement domElement)
{
	QDomNodeList children = domElement.childNodes();
	int length = children.length();
	for (int i = 0; i < length; i++)
	{
		QDomNode node = children.item(i);
		if (node.isElement())
		{
			QDomElement element = node.toElement();
			if (element.tagName() != "item")
				continue;

			itemValues.append(element.attribute("value"));
			itemCaptions.append(element.attribute("caption"));

			addItem(qApp->translate("@default", element.attribute("caption").toUtf8().constData()));
		}
	}

	return ConfigWidget::fromDomElement(domElement);
}
