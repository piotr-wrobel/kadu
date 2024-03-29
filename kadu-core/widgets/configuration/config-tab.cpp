/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
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
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

#include "widgets/configuration/config-group-box.h"
#include "widgets/configuration/config-section.h"
#include "widgets/configuration/kadu-scroll-area.h"

#include "config-tab.h"
#include "moc_config-tab.cpp"

ConfigTab::ConfigTab(const QString &name, ConfigSection *configSection, QWidget *mainWidget)
        : QObject(configSection), MyName(name)
{
    MyScrollArea = new KaduScrollArea(mainWidget);
    MyScrollArea->setFrameStyle(QFrame::NoFrame);
    MyScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    MyScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    MyScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    MyMainWidget = new QWidget(MyScrollArea);
    MyMainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    MyMainLayout = new QVBoxLayout(MyMainWidget);
    MyMainLayout->addStretch(1);

    MyScrollArea->setWidget(MyMainWidget);
    MyScrollArea->setWidgetResizable(true);
}

ConfigTab::~ConfigTab()
{
    /* NOTE: It's needed to call ConfigSection::configTabDestroyed before this
     * ConfigSection will be destroyed. If we relied on QObject to send this signal,
     * it'd be called after destroying all ConfigTab data but we need that data.
     */
    blockSignals(false);
    emit destroyed(this);

    // qDeleteAll() won't work here because of connection to destroyed() signal
    for (auto cgb : MyConfigGroupBoxes)
    {
        disconnect(cgb, SIGNAL(destroyed(QObject *)), this, SLOT(configGroupBoxDestroyed(QObject *)));
        delete cgb;
    }

    delete MyScrollArea;
    MyScrollArea = 0;
}

void ConfigTab::configGroupBoxDestroyed(QObject *obj)
{
    // see ConfigGroupBox::~ConfigGroupBox()
    disconnect(obj, SIGNAL(destroyed(QObject *)), this, SLOT(configGroupBoxDestroyed(QObject *)));

    MyConfigGroupBoxes.remove(static_cast<ConfigGroupBox *>(obj)->name());

    if (MyConfigGroupBoxes.isEmpty())
        deleteLater();
}

ConfigGroupBox *ConfigTab::configGroupBox(const QString &name, bool create)
{
    if (MyConfigGroupBoxes.contains(name))
        return MyConfigGroupBoxes.value(name);

    if (!create)
        return 0;

    QGroupBox *groupBox = new QGroupBox(name, MyMainWidget);
    QHBoxLayout *groupBoxLayout = new QHBoxLayout(groupBox);
    groupBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);

    MyMainLayout->insertWidget(MyConfigGroupBoxes.count(), groupBox);

    ConfigGroupBox *newConfigGroupBox = new ConfigGroupBox(name, this, groupBox);
    MyConfigGroupBoxes.insert(name, newConfigGroupBox);
    connect(newConfigGroupBox, SIGNAL(destroyed(QObject *)), this, SLOT(configGroupBoxDestroyed(QObject *)));

    groupBox->show();

    return newConfigGroupBox;
}

QWidget *ConfigTab::widget() const
{
    return MyScrollArea;
}
