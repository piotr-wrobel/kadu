/*
 * %kadu copyright begin%
 * Copyright 2012 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "jabber-url-dom-visitor-provider.h"
#include "moc_jabber-url-dom-visitor-provider.cpp"

#include "dom/ignore-links-dom-visitor.h"
#include "misc/memory.h"
#include "url-handlers/simple-url-expander.h"

JabberUrlDomVisitorProvider::JabberUrlDomVisitorProvider(QObject *parent)
        : QObject{parent}, m_ignoreLinks{std::make_unique<SimpleUrlExpander>(
                               QRegExp{"\\b"
                                       "xmpp:"
                                       "(?://([^@ ]+)@([^/?# ]+)/?)?"                   // auth-xmpp
                                       "(?:(?:([^@ ]+)@)?([^/?# ]+)(?:/([^?# ]+))?)?"   // path-xmpp
                                       "(?:\\?([^&# ]+)"                                // querytype
                                       "(&[^# ]+)?)?"   // pair, will need to be reparsed, later
                                       "(?:#(\\S*))?"   // fragment
                                       "\\b"})}
{
}

JabberUrlDomVisitorProvider::~JabberUrlDomVisitorProvider()
{
}

const DomVisitor *JabberUrlDomVisitorProvider::provide() const
{
    return &m_ignoreLinks;
}
