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

#pragma once

#include <QtCore/QObject>
#include <QXmppQt5/QXmppClient.h>
#include <QXmppQt5/QXmppStanza.h>

class QXmppIq;

class JabberErrorService : public QObject
{
    Q_OBJECT

public:
    explicit JabberErrorService(QObject *parent = nullptr);
    virtual ~JabberErrorService();

    bool isErrorIq(const QXmppIq &iq) const;
    QString errorMessage(QXmppClient *client, QXmppClient::Error error) const;
    QString errorMessage(const QXmppStanza &stanza, QString conditionString = QString{}) const;
    QString conditionToString(QXmppStanza::Error::Condition condition) const;
};
