/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include "core/core.h"
#include "misc/paths-provider.h"

#include "emoticon-theme-manager.h"
#include "moc_emoticon-theme-manager.cpp"

QString EmoticonThemeManager::defaultTheme()
{
#ifdef Q_OS_LINUX
    return QStringLiteral("penguins");
#else
    return QStringLiteral("tango");
#endif
}

bool EmoticonThemeManager::containsEmotsTxt(const QString &dir)
{
    QString kaduIconFileName = dir + "/emots.txt";
    QFileInfo kaduIconFile(kaduIconFileName);

    return kaduIconFile.exists();
}

EmoticonThemeManager::EmoticonThemeManager(QObject *parent) : ThemeManager(parent)
{
}

EmoticonThemeManager::~EmoticonThemeManager()
{
}

void EmoticonThemeManager::setPathsProvider(PathsProvider *pathsProvider)
{
    m_pathsProvider = pathsProvider;
}

QString EmoticonThemeManager::defaultThemeName() const
{
    return defaultTheme();
}

QStringList EmoticonThemeManager::defaultThemePaths() const
{
    // Allow local themes to override global ones.
    QStringList result = getSubDirs(m_pathsProvider->profilePath() + QStringLiteral("emoticons"));
    result += getSubDirs(m_pathsProvider->dataPath() + QStringLiteral("themes/emoticons"));

    return result;
}

bool EmoticonThemeManager::isValidThemePath(const QString &themePath) const
{
    if (containsEmotsTxt(themePath))
        return true;

    QDir themeDir(themePath);
    QFileInfoList subDirs = themeDir.entryInfoList(QDir::Dirs);

    for (auto const &subDirInfo : subDirs)
    {
        if (!subDirInfo.fileName().startsWith('.'))
            if (containsEmotsTxt(subDirInfo.canonicalFilePath()))
                return true;
    }

    return false;
}
