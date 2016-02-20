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

#include "hints-configuration.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"

#include <QtWidgets/QApplication>

HintsConfiguration::HintsConfiguration(QObject *parent) :
		QObject{parent}
{
}

HintsConfiguration::~HintsConfiguration()
{
}

void HintsConfiguration::setConfiguration(Configuration *configuration)
{
	m_configuration = configuration;
}

void HintsConfiguration::init()
{
	createDefaultConfiguration();
	configurationUpdated();
}

void HintsConfiguration::createDefaultConfiguration()
{
	m_configuration->deprecatedApi()->addVariable("Hints", "MinimumWidth", 285);
	m_configuration->deprecatedApi()->addVariable("Hints", "MaximumWidth", 500);
	m_configuration->deprecatedApi()->addVariable("Hints", "ScreenCorner", static_cast<int>(Corner::BottomRight));

#if !defined(Q_OS_UNIX)
	m_configuration->deprecatedApi()->addVariable("Notify", "ConnectionError_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "NewChat_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "NewMessage_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToFreeForChat_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToOnline_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToAway_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToNotAvailable_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToDoNotDisturb_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToOffline_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "FileTransfer_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "FileTransfer/IncomingFile_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "FileTransfer/Finished_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "multilogon_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "multilogon/sessionConnected_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "multilogon/sessionDisconnected_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "Roster/ImportFailed_UseCustomSettings", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "Roster/ImportFailed_Hints", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "Roster/ExportFailed_UseCustomSettings", true);
	m_configuration->deprecatedApi()->addVariable("Notify", "Roster/ExportFailed_Hints", true);
#endif

	m_configuration->deprecatedApi()->addVariable("Hints", "CiteSign", 50);

	m_configuration->deprecatedApi()->addVariable("Hints", "MouseOverUserSyntax", QString());
	m_configuration->deprecatedApi()->addVariable("Hints", "NewHintUnder", 0);
	m_configuration->deprecatedApi()->addVariable("Hints", "ShowContentMessage", true);

	m_configuration->deprecatedApi()->addVariable("Hints", "AllEvents_iconSize", 32);

	m_configuration->deprecatedApi()->addVariable("Hints", "HintOverUser_iconSize", 32);
	m_configuration->deprecatedApi()->addVariable("Hints", "HintOverUser_font", qApp->font());
	m_configuration->deprecatedApi()->addVariable("Hints", "HintOverUser_Geometry", "50, 50, 640, 610");
	m_configuration->deprecatedApi()->addVariable("Hints", "HintEventConfiguration_Geometry", "50, 50, 520, 345");

	const QString default_hints_syntax(QT_TRANSLATE_NOOP("HintManager", "<table>"
"<tr>"
"<td align=\"left\" valign=\"top\">"
"<img style=\"max-width:64px; max-height:64px;\" "
"src=\"{#{avatarPath} #{avatarPath}}{~#{avatarPath} @{kadu_icons/kadu:64x64}}\""
">"
"</td>"
"<td width=\"100%\">"
"<div>[<b>%a</b>][&nbsp;<b>(%g)</b>]</div>"
"[<div><img height=\"16\" width=\"16\" src=\"#{statusIconPath}\">&nbsp;&nbsp;%u</div>]"
"[<div><img height=\"16\" width=\"16\" src=\"@{phone:16x16}\">&nbsp;&nbsp;%m</div>]"
"[<div><img height=\"16\" width=\"16\" src=\"@{mail-message-new:16x16}\">&nbsp;&nbsp;%e</div>]"
"</td>"
"</tr>"
"</table>"
"[<hr><b>%s</b>][<b>:</b><br><small>%d</small>]"));
	m_configuration->deprecatedApi()->addVariable("Hints", "MouseOverUserSyntax", default_hints_syntax);
}

void HintsConfiguration::configurationUpdated()
{
	m_minimumWidth = qBound(100, m_configuration->deprecatedApi()->readNumEntry("Hints", "MinimumWidth", 285), 10240);
	m_maximumWidth = qBound(100, m_configuration->deprecatedApi()->readNumEntry("Hints", "MaximumWidth", 500), 10240);
	if (m_minimumWidth > m_maximumWidth)
		std::swap(m_minimumWidth, m_maximumWidth);
	m_corner = static_cast<Corner>(m_configuration->deprecatedApi()->readNumEntry("Hints", "ScreenCorner", static_cast<int>(Corner::BottomRight)));
	if (m_corner < Corner::TopLeft || m_corner > Corner::BottomRight)
		m_corner = Corner::BottomRight;

	emit updated();
}

int HintsConfiguration::minimumWidth() const
{
	return m_minimumWidth;
}

int HintsConfiguration::maximumWidth() const
{
	return m_maximumWidth;
}

HintsConfiguration::Corner HintsConfiguration::corner() const
{
	return m_corner;
}

#include "moc_hints-configuration.cpp"
