/*
 * %kadu copyright begin%
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 * Copyright 2011 Wojciech Treter (juzefwt@gmail.com)
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

#include "sql-restore.h"
#include "moc_sql-restore.cpp"

#include "misc/paths-provider.h"

#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

#define RECOVERY_SCRIPT "plugins/data/sql_history/scripts/history-database-recovery.sh"

bool SqlRestore::isCorrupted(const QSqlDatabase &database)
{
    if (!database.isOpen())   // do not restore closed database
        return false;

    if (database.isOpenError())   // restore every database that was not properly opened
        return true;

    const QStringList &tables = database.tables();
    if (QSqlError::NoError != database.lastError().type())
        return true;

    return tables.isEmpty();
}

QString SqlRestore::errorMessage(SqlRestore::RestoreError error)
{
    switch (error)
    {
    case ErrorNoError:
        return tr("No error.");
    case ErrorSqlite3NotExecutable:
        return tr("sqlite3 executable not found.");
    case ErrorInvalidParameters:
        return tr("Invalid invocation of recovery script.");
    case ErrorUnreadableCorruptedDatabase:
    case ErrorInvalidDirectory:
        return tr("Unable to read corrupted database.");
    case ErrorUnableToCreateBackup:
        return tr("Unable to create backup file. Disc may be full.");
    case ErrorNoRestoreScriptExecutable:
        return tr("Recovery script not found or not executable.");
    default:
        return tr("Unknown error during database recovery.");
    }
}

SqlRestore::SqlRestore(QObject *parent) : QObject{parent}
{
}

SqlRestore::~SqlRestore()
{
}

void SqlRestore::setPathsProvider(PathsProvider *pathsProvider)
{
    m_pathsProvider = pathsProvider;
}

SqlRestore::RestoreError SqlRestore::performRestore(const QString &databaseFilePath)
{
    QString recoveryScriptPath = m_pathsProvider->dataPath() + QStringLiteral(RECOVERY_SCRIPT);

    QFileInfo recoveryScriptFileInfo(recoveryScriptPath);
    if (!recoveryScriptFileInfo.exists())
        return ErrorNoRestoreScriptExecutable;

    QProcess restoreProcess;
    restoreProcess.execute("bash", QStringList() << recoveryScriptPath << databaseFilePath);
    restoreProcess.waitForFinished(-1);

    if (restoreProcess.exitCode() < 0 || restoreProcess.exitCode() > ErrorRecovering)
        return ErrorRecovering;

    return static_cast<RestoreError>(restoreProcess.exitCode());
}
