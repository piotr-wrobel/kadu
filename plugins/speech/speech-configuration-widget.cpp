/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
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

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"

#include "speech-configuration-widget.h"
#include "moc_speech-configuration-widget.cpp"

SpeechConfigurationWidget::SpeechConfigurationWidget(QWidget *parent) : NotifierConfigurationWidget(parent)
{
    maleLineEdit = new QLineEdit(this);
    femaleLineEdit = new QLineEdit(this);

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setMargin(0);
    gridLayout->addWidget(new QLabel(tr("Male format") + ':', this), 0, 0, Qt::AlignRight);
    gridLayout->addWidget(maleLineEdit, 0, 1);
    gridLayout->addWidget(new QLabel(tr("Female format") + ':', this), 1, 0, Qt::AlignRight);
    gridLayout->addWidget(femaleLineEdit, 1, 1);

    parent->layout()->addWidget(this);
}

SpeechConfigurationWidget::~SpeechConfigurationWidget()
{
}

void SpeechConfigurationWidget::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void SpeechConfigurationWidget::saveNotifyConfigurations()
{
    if (!currentNotificationEvent.isEmpty())
    {
        maleFormat[currentNotificationEvent] = maleLineEdit->text();
        femaleFormat[currentNotificationEvent] = femaleLineEdit->text();
    }

    QMapIterator<QString, QString> i(maleFormat);
    while (i.hasNext())
    {
        i.next();
        const QString &eventName = i.key();
        m_configuration->deprecatedApi()->writeEntry("Speech", eventName + "_Syntax/Male", i.value());
    }

    QMapIterator<QString, QString> j(femaleFormat);
    while (j.hasNext())
    {
        j.next();
        const QString &eventName = j.key();
        m_configuration->deprecatedApi()->writeEntry("Speech", eventName + "_Syntax/Female", j.value());
    }
}

void SpeechConfigurationWidget::switchToEvent(const QString &event)
{
    if (!currentNotificationEvent.isEmpty())
    {
        maleFormat[currentNotificationEvent] = maleLineEdit->text();
        femaleFormat[currentNotificationEvent] = femaleLineEdit->text();
    }
    currentNotificationEvent = event;

    if (maleFormat.contains(event))
        maleLineEdit->setText(maleFormat[event]);
    else
        maleLineEdit->setText(m_configuration->deprecatedApi()->readEntry("Speech", event + "_Syntax/Male"));

    if (femaleFormat.contains(event))
        femaleLineEdit->setText(femaleFormat[event]);
    else
        femaleLineEdit->setText(m_configuration->deprecatedApi()->readEntry("Speech", event + "_Syntax/Female"));
}
