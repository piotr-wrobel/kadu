/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2008, 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2010, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2007 Marcin Ślusarz (joi@kadu.net)
 * Copyright 2007, 2008 Dawid Stawiarski (neeo@kadu.net)
 * Copyright 2007, 2008, 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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
#include <QtCore/QFile>
#include <QtCore/QSettings>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"

#include "core/core.h"
#include "misc/misc.h"
#include "misc/paths-provider.h"
#include "windows/message-dialog.h"

#include "themes.h"
#include "moc_themes.cpp"

Themes::Themes(const QString &themename, const QString &configname)
        : QObject(), ThemesList(), ThemesPaths(), additional(), ConfigName(configname), Name(themename),
          ActualTheme("Custom"), entries()
{
}

void Themes::setPathsProvider(PathsProvider *pathsProvider)
{
    m_pathsProvider = pathsProvider;
}

void Themes::init()
{
    setPaths(QStringList());
}

QStringList Themes::getSubDirs(const QString &path, bool validate) const
{
    QDir dir(path);
    dir.setFilter(QDir::Dirs);
    QStringList dirs = dir.entryList();
    dirs.removeAll(".");
    dirs.removeAll("..");

    if (!validate)
        return dirs;

    QStringList subdirs;
    for (auto const &dir : dirs)
    {
        QString dirname = path + '/' + dir;
        if (validateDir(dirname))
            subdirs.append(dir);
    }
    return subdirs;
}

bool Themes::validateDir(const QString &path) const
{
    if (ConfigName.isEmpty())
        return true;

    QFile f(path + '/' + ConfigName);
    if (f.exists())
        return true;

    QStringList subdirs = getSubDirs(path, false);
    if (!subdirs.isEmpty())
    {
        for (auto const &dir : subdirs)
        {
            f.setFileName(path + '/' + dir + '/' + ConfigName);
            if (!f.exists())
                return false;
        }

        return true;
    }

    return false;
}

const QStringList &Themes::themes() const
{
    return ThemesList;
}

void Themes::setTheme(const QString &theme)
{
    if (ThemesList.contains(theme) || (theme == "Custom"))
    {
        entries.clear();
        ActualTheme = theme;
        if (theme != "Custom" && !ConfigName.isEmpty())
        {
            QSettings themeSettings(themePath() + fixFileName(themePath(), ConfigName), QSettings::IniFormat);
            themeSettings.setIniCodec("ISO8859-2");

            themeSettings.beginGroup(Name);
            auto keys = themeSettings.allKeys();
            for (auto const &key : keys)
            {
                entries.insert(key, themeSettings.value(key).toString());
            }
            themeSettings.endGroup();
        }
        emit themeChanged(ActualTheme);
    }
}

const QString &Themes::theme() const
{
    return ActualTheme;
}

void Themes::setPaths(const QStringList &paths)
{
    ThemesList.clear();
    ThemesPaths.clear();
    additional.clear();
    QStringList temp = paths + defaultPathsProviderWithThemes();
    for (auto const &it : temp)
    {
        if (validateDir(it))
        {
            if (paths.indexOf(it) != -1)
                additional.append(it);
            ThemesPaths.append(it);
            ThemesList.append(it.section('/', -1, -1, QString::SectionSkipEmpty));
        }
        // TODO: 0.6.5
        // 		else
        // 			MessageDialog::msg(tr("<i>%1</i><br/>does not contain any theme configuration
        // file").arg(it),
        // false, "dialog-warning");
    }
    emit pathsChanged(ThemesPaths);
}

QStringList Themes::defaultPathsProviderWithThemes() const
{
    QStringList result;

    auto path = QString{m_pathsProvider->dataPath() + QStringLiteral("themes/") + Name};
    for (auto const &it : getSubDirs(path))
        result << (path + '/' + it + '/');

    for (auto const &it : getSubDirs(m_pathsProvider->profilePath() + Name))
        result << (m_pathsProvider->profilePath() + Name + '/' + it + '/');

    return result;
}

const QStringList &Themes::paths() const
{
    return ThemesPaths;
}

const QStringList &Themes::additionalPaths() const
{
    return additional;
}

QString Themes::themePath(const QString &theme) const
{
    QString t = theme;
    if (theme.isEmpty())
        t = ActualTheme;
    if (t == "Custom")
        return QString();
    if (ThemesPaths.isEmpty())
        return "Custom";

    QRegExp r("(/" + t + "/)$");
    for (auto const &theme : ThemesPaths)
        if (-1 != r.indexIn(theme))
            return theme;

    return "Custom";
}

QString Themes::getThemeEntry(const QString &name) const
{
    if (entries.contains(name))
        return entries[name];
    else
        return QString();
}
