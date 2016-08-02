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

#include "force-nbsp-dom-visitor.h"

#include <QtXml/QDomElement>
#include <QtXml/QDomText>

ForceNbspDomVisitor::ForceNbspDomVisitor() :
		DomTextRegexpVisitor{QRegExp{"  "}}
{
}

ForceNbspDomVisitor::~ForceNbspDomVisitor()
{
}

QList<QDomNode> ForceNbspDomVisitor::matchToDomNodes(QDomDocument document, QRegExp regExp) const
{
	Q_UNUSED(regExp);

	auto spaceNode = document.createTextNode(" ");
	auto domEntity = document.createEntityReference("nbsp");
	return QList<QDomNode>{} << spaceNode << domEntity;
}