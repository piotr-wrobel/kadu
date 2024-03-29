/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtGui/QImageWriter>

#include "configuration/gui/configuration-ui-handler-repository.h"
#include "core/core.h"
#include "misc/paths-provider.h"

#include "widgets/configuration/config-combo-box.h"
#include "widgets/configuration/configuration-widget.h"
#include "windows/main-configuration-window.h"

#include "screenshot-configuration-ui-handler.h"
#include "moc_screenshot-configuration-ui-handler.cpp"

ScreenShotConfigurationUiHandler::ScreenShotConfigurationUiHandler(QObject *parent) : QObject{parent}
{
}

ScreenShotConfigurationUiHandler::~ScreenShotConfigurationUiHandler()
{
}

void ScreenShotConfigurationUiHandler::mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow)
{
    QObject::connect(
        mainConfigurationWindow->widget()->widgetById("screenshot/enableSizeLimit"), SIGNAL(toggled(bool)),
        mainConfigurationWindow->widget()->widgetById("screenshot/sizeLimit"), SLOT(setEnabled(bool)));

    QStringList opts;
    QList<QByteArray> byteArrayOpts = QImageWriter::supportedImageFormats();

    for (auto const &opt : byteArrayOpts)
        opts.append(QString(opt));

    ConfigComboBox *formats =
        static_cast<ConfigComboBox *>(mainConfigurationWindow->widget()->widgetById("screenshot/formats"));
    if (formats)
        formats->setItems(opts, opts);
}

void ScreenShotConfigurationUiHandler::mainConfigurationWindowDestroyed()
{
}

void ScreenShotConfigurationUiHandler::mainConfigurationWindowApplied()
{
}
