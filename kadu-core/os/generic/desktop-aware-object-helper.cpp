/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "desktop-aware-object-helper.h"
#include "moc_desktop-aware-object-helper.cpp"

#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QWidget>

#include "misc/misc.h"
#include "os/generic/desktop-aware-object.h"

#define DESKTOP_AWARE_OBJECT_HELPER_TIMER_INTERVAL 300 /*ms*/

DesktopAwareObjectHelper::DesktopAwareObjectHelper()
{
    Timer.setInterval(DESKTOP_AWARE_OBJECT_HELPER_TIMER_INTERVAL);
    Timer.setSingleShot(true);
    connect(&Timer, SIGNAL(timeout()), this, SLOT(workAreaResized()));
    connect(QApplication::desktop(), SIGNAL(workAreaResized(int)), &Timer, SLOT(start()));
}

DesktopAwareObjectHelper::~DesktopAwareObjectHelper()
{
    Timer.stop();
}

void DesktopAwareObjectHelper::workAreaResized()
{
    DesktopAwareObject::notifyDesktopModified();
}
