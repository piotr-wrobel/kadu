/*
 * %kadu copyright begin%
 * Copyright 2012 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#ifndef OTR_INSTANCE_TAG_SERVICE_H
#define OTR_INSTANCE_TAG_SERVICE_H

#include <QtCore/QObject>
#include <QtCore/QWeakPointer>

extern "C" {
#	include <libotr/proto.h>
}

class Account;

class OtrUserStateService;

class OtrInstanceTagService : public QObject
{
	Q_OBJECT

	QWeakPointer<OtrUserStateService> UserStateService;

	QString instanceTagsFileName() const;

public:
	static void wrapperOtrCreateInstanceTag(void *data, const char *accountName, const char *protocol);

	explicit OtrInstanceTagService(QObject *parent = 0);
	virtual ~OtrInstanceTagService();

	void setUserStateService(OtrUserStateService *userStateService);

	void readInstanceTags();
	void writeInstanceTags();
	void createInstanceTag(const Account &account);

};

#endif // OTR_INSTANCE_TAG_SERVICE_H
