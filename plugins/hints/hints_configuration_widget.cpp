/*
 * %kadu copyright begin%
 * Copyright 2008, 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2008 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2007, 2008, 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2007 Dawid Stawiarski (neeo@kadu.net)
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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

#include "configuration/configuration-file.h"
#include "gui/widgets/configuration/configuration-widget.h"
#include "gui/widgets/configuration/notify-group-box.h"
#include "activate.h"
#include "debug.h"

#include "hints-configuration-window.h"

#include "hints_configuration_widget.h"

HintsConfigurationWidget::HintsConfigurationWidget(QWidget *parent)
	: NotifierConfigurationWidget(parent)
{
	preview = new QLabel(tr("<b>Here</b> you can see the preview"), this);
	preview->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	preview->setMargin(3);

	QPushButton *configureButton = new QPushButton(tr("Configure"));
	connect(configureButton, SIGNAL(clicked()), this, SLOT(showConfigurationWindow()));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(preview);
	layout->addWidget(configureButton);

	static_cast<NotifyGroupBox *>(parent)->addWidget(this);
}

void HintsConfigurationWidget::saveNotifyConfigurations()
{
}

void HintsConfigurationWidget::switchToEvent(const QString &event)
{
	kdebugf();

	currentNotifyEvent = event;
	updatePreview();
}

void HintsConfigurationWidget::showConfigurationWindow()
{
	HintsConfigurationWindow *configWindow = HintsConfigurationWindow::configWindowForEvent(currentNotifyEvent);
	connect(configWindow, SIGNAL(configurationSaved()), this, SLOT(updatePreview()));

	configWindow->show();
}

void HintsConfigurationWidget::updatePreview()
{
	QFont font(qApp->font());
	QPalette palette(qApp->palette());

	preview->setFont(config_file.readFontEntry("Hints", "Event_" + currentNotifyEvent + "_font", &font));

	QColor bcolor = config_file.readColorEntry("Hints", "Event_" + currentNotifyEvent + "_bgcolor", &palette.window().color());
	QColor fcolor = config_file.readColorEntry("Hints", "Event_" + currentNotifyEvent + "_fgcolor", &palette.windowText().color());
	QString style = QString("* {color:%1; background-color:%2}").arg(fcolor.name(), bcolor.name());
	preview->setStyleSheet(style);
}

#include "moc_hints_configuration_widget.cpp"
