/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>

#include "configuration/gui/configuration-ui-handler-repository.h"
#include "core/core.h"
#include "misc/paths-provider.h"
#include "widgets/configuration/config-group-box.h"
#include "widgets/configuration/configuration-widget.h"
#include "windows/main-configuration-window.h"

#include "antistring-configuration.h"

#include "antistring-configuration-ui-handler.h"
#include "moc_antistring-configuration-ui-handler.cpp"

AntistringConfigurationUiHandler::AntistringConfigurationUiHandler(QObject *parent)
        : QObject{parent}, ConditionListWidget{}, ConditionWidget{}, FactorWidget{}
{
}

AntistringConfigurationUiHandler::~AntistringConfigurationUiHandler()
{
}

void AntistringConfigurationUiHandler::setAntistringConfiguration(AntistringConfiguration *antistringConfiguration)
{
    m_antistringConfiguration = antistringConfiguration;
}

void AntistringConfigurationUiHandler::mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow)
{
    ConfigGroupBox *groupBox = mainConfigurationWindow->widget()->configGroupBox("Chat", "Antistring", "Conditions");

    QWidget *widget = new QWidget(groupBox->widget());
    widget->setContentsMargins(0, 0, 0, 0);

    QGridLayout *layout = new QGridLayout(widget);

    ConditionListWidget = new QListWidget(widget);
    layout->addWidget(ConditionListWidget, 0, 0, 1, 4);

    ConditionWidget = new QLineEdit(widget);
    layout->addWidget(new QLabel(tr("Condition"), widget), 1, 0);
    layout->addWidget(ConditionWidget, 1, 1, 1, 3);

    FactorWidget = new QSpinBox(widget);
    FactorWidget->setMinimum(0);
    FactorWidget->setMaximum(5);
    FactorWidget->setSpecialValueText(tr("Don't use"));
    layout->addWidget(new QLabel(tr("Factor"), widget), 2, 0);
    layout->addWidget(FactorWidget, 2, 1, 1, 3);

    QPushButton *addConditionButton = new QPushButton(tr("Add"), widget);
    QPushButton *changeConditionButton = new QPushButton(tr("Change"), widget);
    QPushButton *deleteConditionButton = new QPushButton(tr("Delete"), widget);
    layout->addWidget(addConditionButton, 3, 1);
    layout->addWidget(changeConditionButton, 3, 2);
    layout->addWidget(deleteConditionButton, 3, 3);

    groupBox->addWidget(widget);

    connect(ConditionListWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(wordSelected(QListWidgetItem *)));
    connect(addConditionButton, SIGNAL(clicked()), this, SLOT(addCondition()));
    connect(changeConditionButton, SIGNAL(clicked()), this, SLOT(changeCondition()));
    connect(deleteConditionButton, SIGNAL(clicked()), this, SLOT(deleteCondition()));

    updateConditionList();

    connect(mainConfigurationWindow, SIGNAL(configurationWindowApplied()), this, SLOT(applyConfiguration()));
}

void AntistringConfigurationUiHandler::mainConfigurationWindowDestroyed()
{
}

void AntistringConfigurationUiHandler::mainConfigurationWindowApplied()
{
}

void AntistringConfigurationUiHandler::updateConditionList()
{
    ConditionListWidget->clear();

    for (auto const &condition : m_antistringConfiguration->conditions())
        ConditionListWidget->addItem(QString("(%1) %2").arg(condition.second).arg(condition.first));
}

void AntistringConfigurationUiHandler::addCondition()
{
    QString condition = ConditionWidget->text();
    int factor = FactorWidget->value();

    if (condition.isEmpty())
        return;

    ConditionListWidget->addItem(QString("(%1) %2").arg(factor).arg(condition));
    m_antistringConfiguration->conditions().append(qMakePair(condition, factor));

    FactorWidget->setValue(0);
    ConditionWidget->clear();
}

void AntistringConfigurationUiHandler::changeCondition()
{
    QListWidgetItem *item = ConditionListWidget->currentItem();
    int index = ConditionListWidget->currentIndex().row();

    QString condition = ConditionWidget->text();
    int factor = FactorWidget->value();

    if (condition.isEmpty())
        return;

    if (index < 0 || index >= m_antistringConfiguration->conditions().count())
        return;

    item->setText(QString("(%1) %2").arg(factor).arg(condition));
    m_antistringConfiguration->conditions()[index] = qMakePair(condition, factor);

    FactorWidget->setValue(0);
    ConditionWidget->clear();
}

void AntistringConfigurationUiHandler::deleteCondition()
{
    int index = ConditionListWidget->currentIndex().row();

    if (index < 0 || index >= m_antistringConfiguration->conditions().count())
        return;

    m_antistringConfiguration->conditions().removeAt(index);

    updateConditionList();
}

void AntistringConfigurationUiHandler::applyConfiguration()
{
    m_antistringConfiguration->storeConditions();
}

void AntistringConfigurationUiHandler::wordSelected(QListWidgetItem *item)
{
    Q_UNUSED(item)

    int index = ConditionListWidget->currentIndex().row();
    if (index < 0 || index >= m_antistringConfiguration->conditions().count())
    {
        FactorWidget->setValue(0);
        ConditionWidget->clear();
        return;
    }

    ConditionPair condition = m_antistringConfiguration->conditions().at(index);

    FactorWidget->setValue(condition.second);
    ConditionWidget->setText(condition.first);
}
