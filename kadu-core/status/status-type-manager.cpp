/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "status-type-manager.h"
#include "moc_status-type-manager.cpp"

#include "icons/kadu-icon.h"
#include "status/status-type-data.h"
#include "status/status-type.h"
#include "status/status.h"

#include <QtCore/QtAlgorithms>
#include <QtWidgets/QApplication>

StatusTypeManager::StatusTypeManager(QObject *parent) : QObject{parent}
{
    m_statusTypes.insert(
        StatusType::FreeForChat,
        StatusTypeData(
            StatusType::FreeForChat, "FreeForChat", QCoreApplication::translate("@default", "Free for chat"),
            "free_for_chat", StatusTypeGroup::Online));
    m_statusTypes.insert(
        StatusType::Online, StatusTypeData(
                                StatusType::Online, "Online", QCoreApplication::translate("@default", "Online"),
                                "online", StatusTypeGroup::Online));
    m_statusTypes.insert(
        StatusType::Away,
        StatusTypeData(
            StatusType::Away, "Away", QCoreApplication::translate("@default", "Away"), "away", StatusTypeGroup::Away));
    m_statusTypes.insert(
        StatusType::NotAvailable,
        StatusTypeData(
            StatusType::NotAvailable, "NotAvailable", QCoreApplication::translate("@default", "Not available"),
            "not_available", StatusTypeGroup::Away));
    m_statusTypes.insert(
        StatusType::DoNotDisturb,
        StatusTypeData(
            StatusType::DoNotDisturb, "DoNotDisturb", QCoreApplication::translate("@default", "Do not disturb"),
            "do_not_disturb", StatusTypeGroup::Away));
    m_statusTypes.insert(
        StatusType::Invisible,
        StatusTypeData(
            StatusType::Invisible, "Invisible", QCoreApplication::translate("@default", "Invisible"), "invisible",
            StatusTypeGroup::Invisible));
    m_statusTypes.insert(
        StatusType::Offline, StatusTypeData(
                                 StatusType::Offline, "Offline", QCoreApplication::translate("@default", "Offline"),
                                 "offline", StatusTypeGroup::Offline));
}

StatusTypeManager::~StatusTypeManager()
{
}

StatusType StatusTypeManager::fromName(const QString &name)
{
    for (QMap<StatusType, StatusTypeData>::const_iterator it = m_statusTypes.constBegin(),
                                                          end = m_statusTypes.constEnd();
         it != end; ++it)
        if (it.value().name() == name)
            return it.key();
    return StatusType::Offline;
}

const StatusTypeData StatusTypeManager::statusTypeData(StatusType statusType)
{
    if (m_statusTypes.contains(statusType))
        return m_statusTypes.value(statusType);
    return m_statusTypes.value(StatusType::Offline);
}

KaduIcon StatusTypeManager::statusIcon(const QString &protocol, const Status &status)
{
    const StatusTypeData &statusTypeData = this->statusTypeData(status.type());

    QString iconName = QString("protocols/%1/%2%3")
                           .arg(protocol)
                           .arg(statusTypeData.iconName())
                           .arg(status.description().isEmpty() ? QString() : QStringLiteral("_d"));

    return KaduIcon(iconName, "16x16");
}
