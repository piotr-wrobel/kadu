/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QTimer>

#include "actions/action-description.h"
#include "actions/action.h"
#include "contacts/contact-set.h"
#include "core/core.h"
#include "menu-item.h"
#include "menu/menu-inventory.h"
#include "protocols/protocol-menu-manager.h"
#include "windows/kadu-window-service.h"
#include "windows/kadu-window.h"

#include "kadu-menu.h"
#include "moc_kadu-menu.cpp"

KaduMenu::KaduMenu(const QString &category, KaduMenu *parent) : QObject{parent}, Category{category}, IsSorted{true}
{
}

KaduMenu::~KaduMenu()
{
}

void KaduMenu::setKaduWindowService(KaduWindowService *kaduWindowService)
{
    m_kaduWindowService = kaduWindowService;
}

void KaduMenu::setMenuInventory(MenuInventory *menuInventory)
{
    m_menuInventory = menuInventory;
}

void KaduMenu::menuDestroyed(QObject *object)
{
    Menus.removeAll(object);
}

void KaduMenu::attachToMenu(QMenu *menu)
{
    if (!menu)
        return;

    Menus.append(menu);
    connect(menu, SIGNAL(destroyed(QObject *)), this, SLOT(menuDestroyed(QObject *)));
}

void KaduMenu::detachFromMenu(QMenu *menu)
{
    if (!menu)
        return;

    Menus.removeAll(menu);
    disconnect(menu, SIGNAL(destroyed(QObject *)), this, SLOT(menuDestroyed(QObject *)));
}

bool KaduMenu::empty() const
{
    return Items.empty();
}

KaduMenu *KaduMenu::addAction(ActionDescription *actionDescription, KaduMenu::MenuSection section, int priority)
{
    Items.append(new MenuItem(actionDescription, section, priority));
    IsSorted = false;

    return this;
}

KaduMenu *KaduMenu::removeAction(ActionDescription *actionDescription)
{
    QList<MenuItem *>::iterator i = Items.begin();

    while (i != Items.end())
    {
        if ((*i)->actionDescription() == actionDescription)
            i = Items.erase(i);
        else
            ++i;
    }

    return this;
}

bool KaduMenu::lessThan(const MenuItem *a, const MenuItem *b)
{
    if (a->section() == b->section())
    {
        return a->priority() > b->priority();
    }

    return a->section() < b->section();
}

void KaduMenu::sort()
{
    if (IsSorted)
        return;

    qSort(Items.begin(), Items.end(), lessThan);
    IsSorted = true;
}

void KaduMenu::appendTo(QMenu *menu, ActionContext *context)
{
    sort();

    ActionContext *actionContext = context ? context : getActionContext();

    auto firstItem = true;
    auto actionsFirstItem = true;
    MenuSection latestSection = KaduMenu::SectionAbout;   // prevent 4.9 from complaining

    QMenu *actions = new QMenu(tr("More Actions..."), menu);

    for (auto menuItem : Items)
    {
        if (!menuItem->actionDescription())
            continue;

        auto isActions = menuItem->section() == KaduMenu::SectionActions ||
                         menuItem->section() == KaduMenu::KaduMenu::SectionActionsGui;
        auto currentMenu = isActions ? actions : menu;
        auto menuFirstItem = isActions ? &actionsFirstItem : &firstItem;

        if (!*menuFirstItem && latestSection != menuItem->section())
            currentMenu->addSeparator();

        auto parent = currentMenu->parent() ? currentMenu->parent() : currentMenu;
        Action *action = menuItem->actionDescription()->createAction(actionContext, parent);
        currentMenu->addAction(action);
        action->checkState();

        latestSection = menuItem->section();
        *menuFirstItem = false;
    }

    if ("buddy-list" != Category)
        return;

    auto isContact = actionContext->roles().contains(ContactRole) && 1 == actionContext->contacts().size();
    auto isOneContactbuddy = actionContext->roles().contains(BuddyRole) && 1 == actionContext->buddies().size() &&
                             1 == actionContext->buddies().begin()->contacts().size();
    if (isContact || isOneContactbuddy)
    {
        for (auto manager : m_menuInventory->protocolMenuManagers())
        {
            Contact contact = *actionContext->contacts().constBegin();
            if (contact.contactAccount().protocolName() != manager->protocolName())
                continue;

            if (!firstItem && !manager->protocolActions().isEmpty())
                actions->addSeparator();

            for (auto actionDescription : manager->protocolActions())
            {
                if (actionDescription)
                {
                    Action *action = actionDescription->createAction(actionContext, menu->parent());
                    actions->addAction(action);
                    action->checkState();
                }
                else
                    actions->addSeparator();
            }
        }
    }

    if (actions->actions().size() > 0)
        menu->addMenu(actions);
    else
        delete actions;
}

void KaduMenu::applyTo(QMenu *menu, ActionContext *context)
{
    menu->clear();
    appendTo(menu, context);
}

void KaduMenu::updateGuiMenuLater()
{
    QTimer::singleShot(1000, this, SLOT(updateGuiMenuSlot()));
}

void KaduMenu::updateGuiMenuSlot()
{
    for (auto menu : Menus)
    {
        auto m = qobject_cast<QMenu *>(menu);
        if (m)
            applyTo(m);
    }
}

ActionContext *KaduMenu::getActionContext()
{
    return m_kaduWindowService->kaduWindow()->actionContext();
}

void KaduMenu::update()
{
    updateGuiMenuLater();
}
