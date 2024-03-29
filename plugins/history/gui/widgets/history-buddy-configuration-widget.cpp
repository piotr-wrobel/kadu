/*
 * %kadu copyright begin%
 * Copyright 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "widgets/simple-configuration-value-state-notifier.h"

#include "history-buddy-configuration-widget.h"
#include "moc_history-buddy-configuration-widget.cpp"

HistoryBuddyConfigurationWidget::HistoryBuddyConfigurationWidget(const Buddy &buddy, QWidget *parent)
        : BuddyConfigurationWidget(buddy, parent), StateNotifier(new SimpleConfigurationValueStateNotifier(this))
{
    setWindowTitle(tr("History"));
}

HistoryBuddyConfigurationWidget::~HistoryBuddyConfigurationWidget()
{
}

void HistoryBuddyConfigurationWidget::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void HistoryBuddyConfigurationWidget::init()
{
    createGui();
    configurationUpdated();
    loadValues();
}

void HistoryBuddyConfigurationWidget::createGui()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    StoreHistoryCheckBox = new QCheckBox(tr("Store history"));

    connect(StoreHistoryCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateState()));

    layout->addWidget(StoreHistoryCheckBox);
    layout->addStretch(100);
}

void HistoryBuddyConfigurationWidget::configurationUpdated()
{
    GlobalStoreHistory = m_configuration->deprecatedApi()->readBoolEntry("History", "SaveChats", true);
    StoreHistoryCheckBox->setEnabled(GlobalStoreHistory);
}

void HistoryBuddyConfigurationWidget::loadValues()
{
    StoreHistoryCheckBox->setChecked(buddy().property("history:StoreHistory", true).toBool());
    StoreHistoryCheckBox->setEnabled(GlobalStoreHistory);
}

void HistoryBuddyConfigurationWidget::updateState()
{
    if (StoreHistoryCheckBox->isChecked() == buddy().property("history:StoreHistory", true).toBool())
        StateNotifier->setState(StateNotChanged);
    else
        StateNotifier->setState(StateChangedDataValid);
}

const ConfigurationValueStateNotifier *HistoryBuddyConfigurationWidget::stateNotifier() const
{
    return StateNotifier;
}

void HistoryBuddyConfigurationWidget::apply()
{
    if (StoreHistoryCheckBox->isChecked())
        buddy().removeProperty("history:StoreHistory");
    else
        buddy().addProperty("history:StoreHistory", false, CustomProperties::Storable);

    updateState();
}

void HistoryBuddyConfigurationWidget::cancel()
{
    loadValues();
}
