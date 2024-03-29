/*
 * %kadu copyright begin%
 * Copyright 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>

#include "accounts/account-manager.h"
#include "plugin/plugin-injected-factory.h"
#include "widgets/protocols-combo-box.h"
#include "windows/your-accounts.h"

#include "config-wizard-choose-network-page.h"
#include "moc_config-wizard-choose-network-page.cpp"

ConfigWizardChooseNetworkPage::ConfigWizardChooseNetworkPage(QWidget *parent)
        : ConfigWizardPage(parent), LastProtocol(0)
{
}

ConfigWizardChooseNetworkPage::~ConfigWizardChooseNetworkPage()
{
}

void ConfigWizardChooseNetworkPage::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void ConfigWizardChooseNetworkPage::init()
{
    setDescription(
        tr("<p>Please choose the network for the account that you would like to set up.</p><p>You can also create a "
           "new account in the wizard if you don't already have one</p>"));
    createGui();
}

void ConfigWizardChooseNetworkPage::createGui()
{
    formLayout()->addRow(new QLabel(tr("<h3>Account Setup</h3>"), this));

    SelectProtocol = m_pluginInjectedFactory->makeInjected<ProtocolsComboBox>(this);
    connect(SelectProtocol, SIGNAL(currentIndexChanged(int)), this, SLOT(protocolChanged()));

    formLayout()->addRow(tr("IM Network"), SelectProtocol);

    SetUpExisting = new QRadioButton(tr("I want to set up existing account for Kadu"), this);
    SetUpNew = new QRadioButton(tr("I want to create new account for Kadu"), this);
    Ignore = new QRadioButton(tr("I don't want to set up my account for Kadu now"), this);

    formLayout()->addRow(QString(), SetUpExisting);
    formLayout()->addRow(QString(), SetUpNew);
    formLayout()->addRow(QString(), Ignore);

    registerField(
        "choose-network.protocol-factory", SelectProtocol, "currentProtocol", SIGNAL(currentIndexChanged(int)));
    registerField("choose-network.existing", SetUpExisting);
    registerField("choose-network.new", SetUpNew);
    registerField("choose-network.ignore", Ignore);

    protocolChanged();
}

void ConfigWizardChooseNetworkPage::protocolChanged()
{
    ProtocolFactory *protocol = SelectProtocol->currentProtocol();
    if (!protocol)
    {
        SetUpExisting->setEnabled(false);
        SetUpNew->setEnabled(false);
        Ignore->setChecked(true);
    }
    else
    {
        SetUpExisting->setEnabled(true);
        SetUpNew->setEnabled(protocol->canRegister());

        if (!LastProtocol || (SetUpNew->isChecked() && !SetUpNew->isEnabled()))
            SetUpExisting->setChecked(true);
    }

    LastProtocol = protocol;
}
