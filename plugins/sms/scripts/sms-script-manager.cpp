/*
 * %kadu copyright begin%
 * Copyright 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#define SMS_USE_DEBUGGER 0

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtScript/QScriptEngine>
#include <QtWidgets/QMainWindow>
#if SMS_USE_DEBUGGER
#include <QtScriptTools/QScriptEngineDebugger>
#endif

#include "misc/paths-provider.h"
#include "plugin/plugin-injected-factory.h"

#include "scripts/network-access-manager-wrapper.h"
#include "scripts/sms-translator.h"

#include "sms-script-manager.h"
#include "moc_sms-script-manager.cpp"

SmsScriptsManager::SmsScriptsManager(QObject *parent) : QObject{parent}
{
}

SmsScriptsManager::~SmsScriptsManager()
{
}

void SmsScriptsManager::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void SmsScriptsManager::setPathsProvider(PathsProvider *pathsProvider)
{
    m_pathsProvider = pathsProvider;
}

void SmsScriptsManager::init()
{
    Engine = new QScriptEngine(this);
    Network = m_pluginInjectedFactory->makeInjected<NetworkAccessManagerWrapper>(Engine, this);

    Engine->globalObject().setProperty("network", Engine->newQObject(Network));
    Engine->globalObject().setProperty("translator", Engine->newQObject(new SmsTranslator(this)));

#if SMS_USE_DEBUGGER
    QScriptEngineDebugger *debugger = new QScriptEngineDebugger(this);
    debugger->attachTo(Engine);
    debugger->standardWindow()->show();
#endif

    QString scriptPath = m_pathsProvider->profilePath() + QStringLiteral("plugins/data/sms/scripts/gateway.js");
    if (QFile::exists(scriptPath))
        loadScript(scriptPath);
    else
    {
        scriptPath = m_pathsProvider->dataPath() + QStringLiteral("plugins/data/sms/scripts/gateway.js");
        if (QFile::exists(scriptPath))
            loadScript(scriptPath);
        // TODO: maybe we should return here if no gateway.js was found?
    }

    // scripts from profile path can replace the ones from data path if the file name is the same
    loadScripts(QDir(m_pathsProvider->profilePath() + QStringLiteral("plugins/data/sms/scripts")));
    loadScripts(QDir(m_pathsProvider->dataPath() + QStringLiteral("plugins/data/sms/scripts")));
}

void SmsScriptsManager::loadScripts(const QDir &dir)
{
    if (!dir.exists())
        return;

    QFileInfoList gateways = dir.entryInfoList(QStringList("gateway-*.js"));
    for (auto const &gatewayFile : gateways)
        loadScript(gatewayFile);
}

void SmsScriptsManager::loadScript(const QFileInfo &fileInfo)
{
    if (!fileInfo.exists())
        return;

    // We want file name exluding the path - file from a higher priority dir can
    // replace a file of the same name from different dir.
    QString fileName = fileInfo.fileName();
    if (LoadedFiles.contains(fileName))
        return;
    LoadedFiles.append(fileName);

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QFile::ReadOnly))
        return;

    QTextStream reader(&file);
    reader.setCodec("UTF-8");
    QString content = reader.readAll();
    file.close();

    if (content.isEmpty())
        return;

    Engine->evaluate(content);
}
