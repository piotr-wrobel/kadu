/*
 * %kadu copyright begin%
 * Copyright 2010 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2010 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2010 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 badboy (badboy@gen2.org)
 * Copyright 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
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

#ifndef TALKABLE_DELEGATE_H
#define TALKABLE_DELEGATE_H

#undef check

#include "buddies/buddy.h"
#include "gui/widgets/talkable-delegate-configuration.h"
#include "gui/widgets/kadu-tree-view-delegate.h"
#include "message/message.h"

class Contact;
class ModelChain;

class TalkableDelegate : public KaduTreeViewDelegate
{
	Q_OBJECT

	ModelChain *Chain;

private slots:
	void contactUpdated(Contact &contact);
	void buddyUpdated(Buddy &buddy);
	void messageStatusChanged(Message message);
	void chainDestroyed();

public:
	explicit TalkableDelegate(TalkableTreeView *parent = 0);
	virtual ~TalkableDelegate();

	virtual void setChain(ModelChain *chain);

};

// for MOC
#include "contacts/contact.h"

#endif // TALKABLE_DELEGATE_H
