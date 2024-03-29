/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "icons-manager.h"
#include "moc_icons-manager.cpp"

#include "accounts/account-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "core/core.h"
#include "icons/kadu-icon.h"
#include "misc/misc.h"
#include "protocols/protocol.h"
#include "themes/icon-theme-manager.h"

#include <QtCore/QFileInfo>

IconsManager::IconsManager(QObject *parent) : QObject{parent}
{
}

IconsManager::~IconsManager()
{
}

void IconsManager::setAccountManager(AccountManager *accountManager)
{
    m_accountManager = accountManager;
}

void IconsManager::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void IconsManager::setIconThemeManager(IconThemeManager *iconThemeManager)
{
    m_iconThemeManager = iconThemeManager;
}

void IconsManager::init()
{
    m_iconThemeManager->loadThemes();
    configurationUpdated();

    // TODO: localized protocol
    localProtocolPath = "gadu-gadu";
}

QString IconsManager::iconPath(const KaduIcon &icon, IconsManager::AllowEmpty allowEmpty) const
{
    QString path = icon.path();
    QString size = icon.size();

    QFileInfo fileInfo(path);
    QString themePath = icon.themePath().isEmpty() ? m_iconThemeManager->currentTheme().path() : icon.themePath();
    QString name = fileInfo.fileName();
    QString realPath = fileInfo.path();

    fileInfo.setFile(themePath + realPath + '/' + size + '/' + name + ".png");
    if (fileInfo.isFile() && fileInfo.isReadable())
        return fileInfo.canonicalFilePath();

    fileInfo.setFile(themePath + realPath + '/' + size + '/' + name + ".gif");
    if (fileInfo.isFile() && fileInfo.isReadable())
        return fileInfo.canonicalFilePath();

    if (realPath == QStringLiteral("protocols/common"))
    {
        QString protocolPath;
        if (m_accountManager->defaultAccount().protocolHandler())
            protocolPath = m_accountManager->defaultAccount().protocolHandler()->statusPixmapPath();
        else
            protocolPath = localProtocolPath;

        KaduIcon protocolPathIcon = icon;
        protocolPathIcon.setPath(QString("protocols/%1/%2").arg(protocolPath).arg(name));
        return iconPath(protocolPathIcon, allowEmpty);
    }

    if (EmptyAllowed == allowEmpty)
        return QString();
    else
        return iconPath(KaduIcon("kadu_icons/0", size), EmptyAllowed);
}

QIcon IconsManager::buildPngIcon(const QString &themePath, const QString &path)
{
    static QString sizes[] = {QStringLiteral("16x16"),  QStringLiteral("22x22"), QStringLiteral("32x32"),
                              QStringLiteral("64x64"),  QStringLiteral("96x96"), QStringLiteral("128x128"),
                              QStringLiteral("256x256")};
    static int sizes_count = 7;

    QIcon icon;
    for (int i = 0; i < sizes_count; i++)
    {
        KaduIcon kaduIcon(path, sizes[i]);
        kaduIcon.setThemePath(themePath);

        QString fullPath = iconPath(kaduIcon, EmptyAllowed);
        if (!fullPath.isEmpty())
            icon.addFile(fullPath);
    }

    return icon;
}

QIcon IconsManager::iconByPath(const QString &themePath, const QString &path, AllowEmpty allowEmpty)
{
    if (!IconCache.contains(themePath + path))
    {
        QIcon icon;

        QFileInfo fileInfo(path);
        if (fileInfo.isAbsolute() && fileInfo.isReadable())
            icon.addFile(path);
        else
        {
            icon = buildPngIcon(themePath, path);

            if (icon.isNull())
            {
                QRegExp commonRegexp = QRegExp("^protocols/common/(.+)$");
                if (path.contains(commonRegexp))
                {
                    QString protocolpath;
                    if (m_accountManager->defaultAccount().protocolHandler())
                        protocolpath = m_accountManager->defaultAccount().protocolHandler()->statusPixmapPath();
                    else
                        protocolpath = localProtocolPath;
                    return iconByPath(themePath, QString("protocols/%1/%2").arg(protocolpath, commonRegexp.cap(1)));
                }
            }

            if (icon.isNull() && EmptyNotAllowed == allowEmpty)
                icon = buildPngIcon(themePath, "kadu_icons/0");
        }

        IconCache.insert(themePath + path, icon);
    }

    return IconCache.value(themePath + path);
}

QIcon IconsManager::iconByPath(const KaduIcon &icon)
{
    return iconByPath(icon.themePath(), icon.path());
}

void IconsManager::clearCache()
{
    IconCache.clear();
}

void IconsManager::configurationUpdated()
{
    bool themeWasChanged =
        m_configuration->deprecatedApi()->readEntry("Look", "IconTheme") != m_iconThemeManager->currentTheme().name();
    if (themeWasChanged)
    {
        clearCache();
        m_iconThemeManager->setCurrentTheme(m_configuration->deprecatedApi()->readEntry("Look", "IconTheme"));
        m_configuration->deprecatedApi()->writeEntry("Look", "IconTheme", m_iconThemeManager->currentTheme().name());

        emit themeChanged();
    }
}

QSize IconsManager::getIconsSize()
{
    return QSize(16, 16);
}
