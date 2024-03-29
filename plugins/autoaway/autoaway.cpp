/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "autoaway.h"
#include "moc_autoaway.cpp"

#include "plugins/idle/idle-plugin-object.h"
#include "plugins/idle/idle.h"

#include "accounts/account.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "configuration/gui/configuration-ui-handler-repository.h"
#include "core/myself.h"
#include "misc/paths-provider.h"
#include "parser/parser.h"
#include "status/status-changer-manager.h"
#include "widgets/configuration/configuration-widget.h"
#include "windows/main-configuration-window.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>

/**
 * @ingroup autoaway
 * @{
 */

Autoaway::Autoaway(QObject *parent)
        : QObject{parent}, m_checkInterval{}, m_autoAwayTime{}, m_autoExtendedAwayTime{}, m_autoDisconnectTime{},
          m_autoInvisibleTime{}, m_autoAwayEnabled{}, m_autoExtendedAwayEnabled{}, m_autoInvisibleEnabled{},
          m_autoDisconnectEnabled{}, m_parseAutoStatus{}, m_statusChanged{false}, m_idleTime{}, m_refreshStatusTime{},
          m_refreshStatusInterval{}, m_changeTo{AutoawayStatusChanger::NoChangeDescription}
{
    m_timer = make_owned<QTimer>(this);
    m_timer->setSingleShot(true);
    connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(checkIdleTime()));
}

Autoaway::~Autoaway()
{
}

void Autoaway::setAutoawayStatusChanger(AutoawayStatusChanger *autoawayStatusChanger)
{
    m_autoawayStatusChanger = autoawayStatusChanger;
}

void Autoaway::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void Autoaway::setIdle(Idle *idle)
{
    m_idle = idle;
}

void Autoaway::setMyself(Myself *myself)
{
    m_myself = myself;
}

void Autoaway::setParser(Parser *parser)
{
    m_parser = parser;
}

void Autoaway::init()
{
    createDefaultConfiguration();
    configurationUpdated();
}

AutoawayStatusChanger::ChangeStatusTo Autoaway::changeStatusTo()
{
    m_idleTime = m_idle->secondsIdle();

    if (m_idleTime >= m_autoDisconnectTime && m_autoDisconnectEnabled)
        return AutoawayStatusChanger::ChangeStatusToOffline;
    else if (m_idleTime >= m_autoInvisibleTime && m_autoInvisibleEnabled)
        return AutoawayStatusChanger::ChangeStatusToInvisible;
    else if (m_idleTime >= m_autoExtendedAwayTime && m_autoExtendedAwayEnabled)
        return AutoawayStatusChanger::ChangeStatusToExtendedAway;
    else if (m_idleTime >= m_autoAwayTime && m_autoAwayEnabled)
        return AutoawayStatusChanger::ChangeStatusToAway;
    else
        return AutoawayStatusChanger::NoChangeStatus;
}

AutoawayStatusChanger::ChangeDescriptionTo Autoaway::changeDescriptionTo()
{
    AutoawayStatusChanger::ChangeStatusTo currentChangeStatusTo = changeStatusTo();
    if (AutoawayStatusChanger::NoChangeStatus == currentChangeStatusTo)
        return AutoawayStatusChanger::NoChangeDescription;

    return m_changeTo;
}

QString Autoaway::descriptionAddon() const
{
    return m_descriptionAddon;
}

void Autoaway::checkIdleTime()
{
    m_idleTime = m_idle->secondsIdle();

    if (m_refreshStatusInterval > 0 && m_idleTime >= m_refreshStatusTime)
    {
        m_descriptionAddon = parseDescription(m_autoStatusText);
        m_refreshStatusTime = m_idleTime + m_refreshStatusInterval;
    }

    if (changeStatusTo() != AutoawayStatusChanger::NoChangeStatus)
    {
        m_autoawayStatusChanger->update();
        m_statusChanged = true;
    }
    else if (m_statusChanged)
    {
        m_statusChanged = false;
        m_autoawayStatusChanger->update();
    }

    m_timer->setInterval(m_checkInterval * 1000);
    m_timer->setSingleShot(true);
    m_timer->start();
}

QString Autoaway::changeDescription(const QString &oldDescription)
{
    return oldDescription;
}

void Autoaway::configurationUpdated()
{
    if (!m_configuration)
        return;

    m_checkInterval = m_configuration->deprecatedApi()->readUnsignedNumEntry("General", "AutoAwayCheckTime");
    m_refreshStatusTime = m_configuration->deprecatedApi()->readUnsignedNumEntry("General", "AutoRefreshStatusTime");
    m_autoAwayTime = m_configuration->deprecatedApi()->readUnsignedNumEntry("General", "AutoAwayTimeMinutes") * 60;
    m_autoExtendedAwayTime =
        m_configuration->deprecatedApi()->readUnsignedNumEntry("General", "AutoExtendedAwayTimeMinutes") * 60;
    m_autoDisconnectTime =
        m_configuration->deprecatedApi()->readUnsignedNumEntry("General", "AutoDisconnectTimeMinutes") * 60;
    m_autoInvisibleTime =
        m_configuration->deprecatedApi()->readUnsignedNumEntry("General", "AutoInvisibleTimeMinutes") * 60;

    m_autoAwayEnabled = m_configuration->deprecatedApi()->readBoolEntry("General", "AutoAway");
    m_autoExtendedAwayEnabled = m_configuration->deprecatedApi()->readBoolEntry("General", "AutoExtendedAway");
    m_autoInvisibleEnabled = m_configuration->deprecatedApi()->readBoolEntry("General", "AutoInvisible");
    m_autoDisconnectEnabled = m_configuration->deprecatedApi()->readBoolEntry("General", "AutoDisconnect");
    m_parseAutoStatus = m_configuration->deprecatedApi()->readBoolEntry("General", "ParseStatus");

    m_refreshStatusInterval = m_refreshStatusTime;

    m_autoStatusText = m_configuration->deprecatedApi()->readEntry("General", "AutoStatusText");
    m_descriptionAddon = parseDescription(m_autoStatusText);

    m_changeTo = (AutoawayStatusChanger::ChangeDescriptionTo)m_configuration->deprecatedApi()->readNumEntry(
        "General", "AutoChangeDescription");

    m_autoawayStatusChanger->update();

    if (m_autoAwayEnabled || m_autoExtendedAwayEnabled || m_autoInvisibleEnabled || m_autoDisconnectEnabled)
    {
        m_timer->setInterval(m_checkInterval * 1000);
        m_timer->setSingleShot(true);
        m_timer->start();
    }
    else
        m_timer->stop();
}

QString Autoaway::parseDescription(const QString &parseDescription)
{
    if (m_parseAutoStatus)
        return (m_parser->parse(parseDescription, Talkable(m_myself->buddy()), ParserEscape::HtmlEscape));
    else
        return parseDescription;
}

static int denominatedInverval(Configuration *configuration, const QString &name, unsigned int def)
{
    int ret = configuration->deprecatedApi()->readUnsignedNumEntry("General", name, def * 60);
    // This AutoawayTimesDenominated thing was living shortly in 1.0-git.
    return configuration->deprecatedApi()->readBoolEntry("General", "AutoawayTimesDenominated", false)
               ? ret
               : (ret + 59) / 60;
}

void Autoaway::createDefaultConfiguration()
{
    m_configuration->deprecatedApi()->addVariable("General", "Autoaway", true);
    m_configuration->deprecatedApi()->addVariable("General", "AutoawayCheckTime", 10);
    m_configuration->deprecatedApi()->addVariable(
        "General", "AutoawayTimeMinutes", denominatedInverval(m_configuration, "AutoawayTime", 5));
    m_configuration->deprecatedApi()->addVariable("General", "AutoExtendedAway", true);
    m_configuration->deprecatedApi()->addVariable(
        "General", "AutoExtendedAwayTimeMinutes", denominatedInverval(m_configuration, "AutoExtendedAwayTime", 15));
    m_configuration->deprecatedApi()->addVariable("General", "AutoChangeDescription", 0);
    m_configuration->deprecatedApi()->addVariable("General", "AutoDisconnect", false);
    m_configuration->deprecatedApi()->addVariable(
        "General", "AutoDisconnectTimeMinutes", denominatedInverval(m_configuration, "AutoDisconnectTime", 60));
    m_configuration->deprecatedApi()->addVariable("General", "AutoInvisible", false);
    m_configuration->deprecatedApi()->addVariable(
        "General", "AutoInvisibleTimeMinutes", denominatedInverval(m_configuration, "AutoInvisibleTime", 30));
    m_configuration->deprecatedApi()->addVariable("General", "AutoRefreshStatusTime", 0);
    m_configuration->deprecatedApi()->addVariable("General", "AutoStatusText", QString());

    // AutoawayCheckTime has been mistakenly denominated in 1.0-git.
    if (0 == m_configuration->deprecatedApi()->readUnsignedNumEntry("General", "AutoawayCheckTime"))
        m_configuration->deprecatedApi()->writeEntry("General", "AutoawayCheckTime", 10);
}

/** @} */
