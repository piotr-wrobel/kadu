/*
 * %kadu copyright begin%
 * Copyright 2008 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2008, 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2007, 2008, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2007, 2008 Dawid Stawiarski (neeo@kadu.net)
 * Copyright 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "gui/widgets/composite-configuration-value-state-notifier.h"
#include "gui/widgets/simple-configuration-value-state-notifier.h"

#include "account-configuration-widget-factory-repository.h"

#include "account-edit-widget.h"
#include "account-configuration-widget-factory.h"

AccountEditWidget::AccountEditWidget(AccountConfigurationWidgetFactoryRepository *accountConfigurationWidgetFactoryRepository, Account account, QWidget *parent) :
		AccountConfigurationWidget(account, parent), MyAccountConfigurationWidgetFactoryRepository(accountConfigurationWidgetFactoryRepository),
		StateNotifier(new SimpleConfigurationValueStateNotifier(this)),
		CompositeStateNotifier(new CompositeConfigurationValueStateNotifier(this))
{
	compositeStateNotifier()->addConfigurationValueStateNotifier(StateNotifier);

	if (MyAccountConfigurationWidgetFactoryRepository)
	{
		connect(MyAccountConfigurationWidgetFactoryRepository, SIGNAL(factoryRegistered(AccountConfigurationWidgetFactory*)),
				this, SLOT(factoryRegistered(AccountConfigurationWidgetFactory*)));
		connect(MyAccountConfigurationWidgetFactoryRepository, SIGNAL(factoryUnregistered(AccountConfigurationWidgetFactory*)),
				this, SLOT(factoryUnregistered(AccountConfigurationWidgetFactory*)));

		foreach (AccountConfigurationWidgetFactory *factory, MyAccountConfigurationWidgetFactoryRepository->factories())
			factoryRegistered(factory);
	}
}

AccountEditWidget::~AccountEditWidget()
{
}

AccountConfigurationWidgetFactoryRepository * AccountEditWidget::accountConfigurationWidgetFactoryRepository() const
{
	return MyAccountConfigurationWidgetFactoryRepository;
}

void AccountEditWidget::factoryRegistered(AccountConfigurationWidgetFactory *factory)
{
	AccountConfigurationWidget *widget = factory->createWidget(account(), this);
	if (widget)
	{
		if (widget->stateNotifier())
			CompositeStateNotifier->addConfigurationValueStateNotifier(widget->stateNotifier());
		AccountConfigurationWidgets.insert(factory, widget);
		emit widgetAdded(widget);
	}
}

void AccountEditWidget::factoryUnregistered(AccountConfigurationWidgetFactory *factory)
{
	if (AccountConfigurationWidgets.contains(factory))
	{
		AccountConfigurationWidget *widget = AccountConfigurationWidgets.value(factory);
		if (widget->stateNotifier())
			CompositeStateNotifier->removeConfigurationValueStateNotifier(widget->stateNotifier());
		emit widgetRemoved(widget);
		widget->deleteLater();
	}
}

QList<AccountConfigurationWidget *> AccountEditWidget::accountConfigurationWidgets() const
{
	return AccountConfigurationWidgets.values();
}

void AccountEditWidget::applyAccountConfigurationWidgets()
{
	foreach (AccountConfigurationWidget *widget, AccountConfigurationWidgets)
		widget->apply();
}

void AccountEditWidget::cancelAccountConfigurationWidgets()
{
	foreach (AccountConfigurationWidget *widget, AccountConfigurationWidgets)
		widget->cancel();
}

SimpleConfigurationValueStateNotifier * AccountEditWidget::simpleStateNotifier() const
{
	return StateNotifier;
}

CompositeConfigurationValueStateNotifier * AccountEditWidget::compositeStateNotifier() const
{
	return CompositeStateNotifier;
}

const ConfigurationValueStateNotifier * AccountEditWidget::stateNotifier() const
{
	return CompositeStateNotifier;
}

#include "moc_account-edit-widget.cpp"
