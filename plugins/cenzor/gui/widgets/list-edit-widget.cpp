/*
 * %kadu copyright begin%
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

#include "list-edit-widget.h"
#include "moc_list-edit-widget.cpp"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>

ListEditWidget::ListEditWidget(QWidget *parent) : QWidget(parent)
{
    createGui();
}

ListEditWidget::~ListEditWidget()
{
}

void ListEditWidget::createGui()
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(5);

    ListWidget = new QListWidget(this);
    layout->addWidget(ListWidget, 0, 0, 1, 4);

    LineEdit = new QLineEdit(this);
    layout->addWidget(LineEdit, 1, 0);

    QPushButton *addItemButton = new QPushButton(tr("Add"), this);
    QPushButton *changeItemButton = new QPushButton(tr("Change"), this);
    QPushButton *deleteItemButton = new QPushButton(tr("Delete"), this);
    layout->addWidget(addItemButton, 1, 1);
    layout->addWidget(changeItemButton, 1, 2);
    layout->addWidget(deleteItemButton, 1, 3);

    connect(
        ListWidget->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this,
        SLOT(selectedItemChanged(QModelIndex, QModelIndex)));
    connect(addItemButton, SIGNAL(clicked()), this, SLOT(addItem()));
    connect(changeItemButton, SIGNAL(clicked()), this, SLOT(changeItem()));
    connect(deleteItemButton, SIGNAL(clicked()), this, SLOT(deleteItem()));
}

void ListEditWidget::setList(const QStringList &list)
{
    ListWidget->clear();
    for (auto const &item : list)
        ListWidget->addItem(item);
}

QStringList ListEditWidget::list()
{
    QStringList result;

    int count = ListWidget->count();
    for (int i = 0; i < count; i++)
        result.append(ListWidget->item(i)->text());

    return result;
}

void ListEditWidget::selectedItemChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (previous != current)
        LineEdit->setText(current.data().toString());
}

void ListEditWidget::addItem()
{
    if (LineEdit->text().isEmpty())
        return;

    ListWidget->addItem(LineEdit->text());
    LineEdit->clear();
}

void ListEditWidget::changeItem()
{
    QListWidgetItem *item = ListWidget->currentItem();
    if (!item)
        return;

    item->setText(LineEdit->text());
    LineEdit->clear();
}

void ListEditWidget::deleteItem()
{
    QListWidgetItem *item = ListWidget->takeItem(ListWidget->currentRow());
    if (!item)
        return;

    delete item;
    LineEdit->clear();
}
