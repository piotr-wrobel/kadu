/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "identities-combo-box.h"
#include "moc_identities-combo-box.cpp"

#include "core/injected-factory.h"
#include "identities/identity-manager.h"
#include "identities/model/identity-model.h"
#include "model/model-chain.h"
#include "model/roles.h"
#include "windows/message-dialog.h"

#include <QtWidgets/QAction>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLineEdit>

IdentitiesComboBox::IdentitiesComboBox(QWidget *parent) : ActionsComboBox(parent)
{
}

IdentitiesComboBox::~IdentitiesComboBox()
{
    m_identityManager->removeUnused();
}

void IdentitiesComboBox::setIdentityManager(IdentityManager *identityManager)
{
    m_identityManager = identityManager;
}

void IdentitiesComboBox::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void IdentitiesComboBox::init()
{
    m_identityManager->removeUnused();

    CreateNewIdentityAction = new QAction(tr("Create a new identity..."), this);
    QFont createNewIdentityActionFont = CreateNewIdentityAction->font();
    createNewIdentityActionFont.setItalic(true);
    CreateNewIdentityAction->setFont(createNewIdentityActionFont);
    CreateNewIdentityAction->setData(true);
    connect(CreateNewIdentityAction, SIGNAL(triggered()), this, SLOT(createNewIdentity()));
    addAfterAction(CreateNewIdentityAction);

    ModelChain *chain = new ModelChain(this);
    chain->setBaseModel(m_injectedFactory->makeInjected<IdentityModel>(chain));
    setUpModel(IdentityRole, chain);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

void IdentitiesComboBox::setCurrentIdentity(Identity identity)
{
    setCurrentValue(identity);
}

Identity IdentitiesComboBox::currentIdentity()
{
    return currentValue().value<Identity>();
}

void IdentitiesComboBox::createNewIdentity()
{
    bool ok;

    QString identityName = QInputDialog::getText(
        this, tr("New Identity"), tr("Please enter the name for the new identity:"), QLineEdit::Normal, QString(), &ok);

    if (!ok)
        return;

    Identity newIdentity = m_identityManager->byName(identityName, true);
    if (newIdentity)
        setCurrentIdentity(newIdentity);
}
