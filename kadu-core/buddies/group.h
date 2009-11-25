/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GROUP_H
#define GROUP_H

#include "buddies/group-shared.h"

#include "storage/shared-base.h"

class Group : public SharedBase<GroupShared>
{
	explicit Group(bool null);

public:
	static Group loadFromStorage(StoragePoint *storage);
	static Group null;

	Group();
	Group(GroupShared *data);
	Group(QObject *data);
	Group(const Group&copy);
	virtual ~Group();

	KaduSharedBase_Property(QString, name, Name)
	KaduSharedBase_Property(QString, icon, Icon)
	KaduSharedBase_Property(bool, notifyAboutStatusChanges, NotifyAboutStatusChanges)
	KaduSharedBase_Property(bool, showInAllGroup, ShowInAllGroup)
	KaduSharedBase_Property(bool, offlineToGroup, OfflineToGroup)
	KaduSharedBase_Property(bool, showIcon, ShowIcon)
	KaduSharedBase_Property(bool, showName, ShowName)
	KaduSharedBase_Property(int, tabPosition, TabPosition)

};

Q_DECLARE_METATYPE(Group)

#endif // GROUP_H
