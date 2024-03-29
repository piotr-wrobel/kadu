/*
 * %kadu copyright begin%
 * Copyright 2016 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "merge-buddies-action.h"
#include "moc_merge-buddies-action.cpp"

#include "actions/action-context.h"
#include "actions/action.h"
#include "buddies/buddy-set.h"
#include "core/injected-factory.h"
#include "core/myself.h"
#include "widgets/dialog/merge-buddies-dialog-widget.h"
#include "windows/kadu-dialog.h"

MergeBuddiesAction::MergeBuddiesAction(QObject *parent)
        :   // using C++ initializers breaks Qt's lupdate
          ActionDescription(parent)
{
    setIcon(KaduIcon{"kadu_icons/merge-buddies"});
    setName(QStringLiteral("mergeContactAction"));
    setText(tr("Merge Buddies..."));
    setType(ActionDescription::TypeUser);
}

MergeBuddiesAction::~MergeBuddiesAction()
{
}

void MergeBuddiesAction::setMyself(Myself *myself)
{
    m_myself = myself;
}

void MergeBuddiesAction::actionTriggered(QAction *sender, bool)
{
    auto action = qobject_cast<Action *>(sender);
    if (!action)
        return;

    auto const &buddy = action->context()->buddies().toBuddy();
    if (!buddy)
        return;

    auto *mergeWidget = injectedFactory()->makeInjected<MergeBuddiesDialogWidget>(
        buddy, tr("Choose which buddy would you like to merge with <i>%1</i>").arg(buddy.display()),
        sender->parentWidget());
    auto window = new KaduDialog(mergeWidget, sender->parentWidget());
    window->setAcceptButtonText(tr("Merge"));
    window->exec();
}

void MergeBuddiesAction::updateActionState(Action *action)
{
    if (action->context()->buddies().isAnyTemporary())
    {
        action->setEnabled(false);
        return;
    }

    if (action->context()->buddies().contains(m_myself->buddy()))
        action->setEnabled(false);
    else
        action->setEnabled(true);

    if (1 != action->context()->buddies().size())
        action->setEnabled(false);
}
