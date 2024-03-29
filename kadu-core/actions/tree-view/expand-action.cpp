/*
 * %kadu copyright begin%
 * Copyright 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "expand-action.h"
#include "moc_expand-action.cpp"

#include "actions/action-context.h"
#include "actions/action.h"

#include <QtWidgets/QTreeView>

ExpandAction::ExpandAction(QObject *parent)
        :   // using C++ initializers breaks Qt's lupdate
          ActionDescription(parent)
{
    setType(ActionDescription::TypeUserList);
    setName("expandAction");
    setText(tr("Expand"));
}

ExpandAction::~ExpandAction()
{
}

void ExpandAction::triggered(QWidget *widget, ActionContext *context, bool toggled)
{
    Q_UNUSED(widget)
    Q_UNUSED(toggled)

    auto treeViewWidget = qobject_cast<QTreeView *>(context->widget());
    if (!treeViewWidget)
        return;

    auto selectedIndexes = treeViewWidget->selectionModel()->selectedIndexes();
    for (auto &&selectedIndex : selectedIndexes)
        treeViewWidget->expand(selectedIndex);
}

void ExpandAction::updateActionState(Action *action)
{
    action->setEnabled(false);

    auto treeViewWidget = qobject_cast<QTreeView *>(action->context()->widget());
    if (!treeViewWidget)
        return;

    auto selectedIndexes = treeViewWidget->selectionModel()->selectedIndexes();
    for (auto &&selectedIndex : selectedIndexes)
        if (treeViewWidget->model()->rowCount(selectedIndex) > 0)
        {
            action->setEnabled(true);
            return;
        }
}
