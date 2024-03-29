/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010 Maciej Płaza (plaza.maciej@gmail.com)
 * Copyright 2010, 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>

#include "core/injected-factory.h"
#include "icons/icons-manager.h"
#include "identities/identity.h"
#include "model/roles.h"
#include "widgets/buddy-contacts-table-delegate.h"
#include "widgets/buddy-contacts-table-item.h"
#include "widgets/buddy-contacts-table-model-proxy.h"
#include "widgets/buddy-contacts-table-model.h"
#include "windows/message-dialog.h"

#include "buddy-contacts-table.h"
#include "moc_buddy-contacts-table.cpp"

BuddyContactsTable::BuddyContactsTable(Buddy buddy, QWidget *parent) : QWidget(parent), MyBuddy(buddy)
{
}

BuddyContactsTable::~BuddyContactsTable()
{
}

void BuddyContactsTable::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void BuddyContactsTable::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void BuddyContactsTable::init()
{
    Delegate = new BuddyContactsTableDelegate(this);
    Model = m_injectedFactory->makeInjected<BuddyContactsTableModel>(MyBuddy, this);
    Proxy = new BuddyContactsTableModelProxy(Model);
    Proxy->setSourceModel(Model);

    createGui();
}

void BuddyContactsTable::createGui()
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    View = new QTableView(this);
    View->setAlternatingRowColors(true);
    View->setDragEnabled(true);
    View->setEditTriggers(QAbstractItemView::AllEditTriggers);
    View->setItemDelegate(Delegate);
    View->setModel(Proxy);

    View->setSelectionBehavior(QAbstractItemView::SelectRows);
    View->setVerticalHeader(0);

    View->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    View->horizontalHeader()->setStretchLastSection(true);

    connect(
        View->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this,
        SLOT(viewSelectionChanged(QModelIndex, QModelIndex)));

    layout->addWidget(View);

    QWidget *buttons = new QWidget(View);
    QVBoxLayout *buttonsLayout = new QVBoxLayout(buttons);

    MoveUpButton = new QPushButton(tr("Move up"), buttons);
    connect(MoveUpButton, SIGNAL(clicked(bool)), this, SLOT(moveUpClicked()));
    buttonsLayout->addWidget(MoveUpButton);

    MoveDownButton = new QPushButton(tr("Move down"), buttons);
    connect(MoveDownButton, SIGNAL(clicked(bool)), this, SLOT(moveDownClicked()));
    buttonsLayout->addWidget(MoveDownButton);

    AddContactButton = new QPushButton(tr("Add contact"), buttons);
    connect(AddContactButton, SIGNAL(clicked(bool)), this, SLOT(addClicked()));
    buttonsLayout->addWidget(AddContactButton);

    DetachContactButton = new QPushButton(tr("Detach contact"), buttons);
    connect(DetachContactButton, SIGNAL(clicked(bool)), this, SLOT(detachClicked()));
    buttonsLayout->addWidget(DetachContactButton);

    RemoveContactButton = new QPushButton(tr("Remove contact"), buttons);
    connect(RemoveContactButton, SIGNAL(clicked(bool)), this, SLOT(removeClicked()));
    buttonsLayout->addWidget(RemoveContactButton);

    viewSelectionChanged(QModelIndex(), QModelIndex());
    layout->addWidget(buttons);
}

const ConfigurationValueStateNotifier *BuddyContactsTable::valueStateNotifier() const
{
    return Model->valueStateNotifier();
}

void BuddyContactsTable::save()
{
    Model->save();
}

void BuddyContactsTable::viewSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    if (!current.isValid())
    {
        MoveUpButton->setEnabled(false);
        MoveDownButton->setEnabled(false);
        DetachContactButton->setEnabled(false);
        RemoveContactButton->setEnabled(false);
    }
    else
    {
        MoveUpButton->setEnabled(current.sibling(current.row() - 1, current.column()).isValid());
        MoveDownButton->setEnabled(current.sibling(current.row() + 1, current.column()).isValid());
        DetachContactButton->setEnabled(true);
        RemoveContactButton->setEnabled(true);
    }

    DetachContactButton->setEnabled(MyBuddy.contacts().count() > 1);
}

void BuddyContactsTable::moveUpClicked()
{
    QModelIndex currentItem = View->currentIndex();
    QModelIndex previousItem = currentItem.sibling(currentItem.row() - 1, currentItem.column());
    if (!previousItem.isValid())
        return;

    BuddyContactsTableItem *current = currentItem.data(BuddyContactsTableItemRole).value<BuddyContactsTableItem *>();
    BuddyContactsTableItem *previous = previousItem.data(BuddyContactsTableItemRole).value<BuddyContactsTableItem *>();

    if (!current || !previous)
        return;

    int priority = current->itemContactPriority();
    current->setItemContactPriority(previous->itemContactPriority());
    previous->setItemContactPriority(priority);

    viewSelectionChanged(View->currentIndex(), previousItem);
}

void BuddyContactsTable::moveDownClicked()
{
    QModelIndex currentItem = View->currentIndex();
    QModelIndex nextItem = currentItem.sibling(currentItem.row() + 1, currentItem.column());
    if (!nextItem.isValid())
        return;

    BuddyContactsTableItem *current = currentItem.data(BuddyContactsTableItemRole).value<BuddyContactsTableItem *>();
    BuddyContactsTableItem *next = nextItem.data(BuddyContactsTableItemRole).value<BuddyContactsTableItem *>();

    if (!current || !next)
        return;

    int priority = current->itemContactPriority();
    current->setItemContactPriority(next->itemContactPriority());
    next->setItemContactPriority(priority);

    viewSelectionChanged(View->currentIndex(), nextItem);
}

void BuddyContactsTable::addClicked()
{
    Model->insertRow(Model->rowCount());
}

void BuddyContactsTable::detachClicked()
{
    BuddyContactsTableItem *item =
        View->currentIndex().data(BuddyContactsTableItemRole).value<BuddyContactsTableItem *>();
    if (!item)
        return;

    QString display = QString("%1 (%2)").arg(item->id()).arg(item->itemAccount().accountIdentity().name());
    display = QInputDialog::getText(
        this, tr("New buddy display name"), tr("Give name for new buddy for this contact"), QLineEdit::Normal, display);

    if (display.isEmpty())
        return;

    item->setAction(BuddyContactsTableItem::ItemDetach);
    item->setDetachedBuddyName(display);
}

void BuddyContactsTable::removeClicked()
{
    BuddyContactsTableItem *item =
        View->currentIndex().data(BuddyContactsTableItemRole).value<BuddyContactsTableItem *>();
    if (!item)
        return;

    if (item->action() == BuddyContactsTableItem::ItemAdd)
    {
        // remove it, we don't need it anyway
        Model->removeRow(View->currentIndex().row());
        return;
    }

    MessageDialog *dialog = MessageDialog::create(
        m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Kadu"),
        tr("Are you sure do you want to delete this contact from buddy <b>%1</b>?").arg(MyBuddy.display()));
    dialog->addButton(QMessageBox::Yes, tr("Delete contact"));
    dialog->addButton(QMessageBox::No, tr("Cancel"));

    if (dialog->ask())
        item->setAction(BuddyContactsTableItem::ItemRemove);
}
