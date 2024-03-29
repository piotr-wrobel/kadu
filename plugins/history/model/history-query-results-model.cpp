/*
 * %kadu copyright begin%
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "chat/chat.h"
#include "model/roles.h"
#include "talkable/talkable-converter.h"

#include "history-query-result.h"
#include "history.h"

#include "history-query-results-model.h"
#include "moc_history-query-results-model.cpp"

HistoryQueryResultsModel::HistoryQueryResultsModel(QObject *parent) : QAbstractListModel(parent)
{
    TalkableHeader = tr("Chat");
    LengthHeader = tr("Length");
}

HistoryQueryResultsModel::~HistoryQueryResultsModel()
{
}

void HistoryQueryResultsModel::setTalkableConverter(TalkableConverter *talkableConverter)
{
    m_talkableConverter = talkableConverter;
}

int HistoryQueryResultsModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 4;
}

int HistoryQueryResultsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : Results.size();
}

QVariant HistoryQueryResultsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (section)
    {
    case 0:
        return TalkableHeader;
    case 1:
        return tr("Date");
    case 2:
        return LengthHeader;
    case 3:
        return tr("Title");
    }

    return QVariant();
}

QVariant HistoryQueryResultsModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();
    int row = index.row();

    if (row < 0 || row >= Results.size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (col)
        {
        case 0:
            return m_talkableConverter->toDisplay(Results.at(row).talkable());
        case 1:
            return Results.at(row).date().toString("dd.MM.yyyy");
        case 2:
            return Results.at(row).count();
        case 3:
            return Results.at(row).title();
        }

        return QVariant();
    }

    case DateRole:
        return Results.at(row).date();
    case TalkableRole:
        return QVariant::fromValue(Results.at(row).talkable());
    }

    return QVariant();
}

void HistoryQueryResultsModel::setTalkableHeader(const QString &talkableHeader)
{
    if (TalkableHeader == talkableHeader)
        return;

    TalkableHeader = talkableHeader;
    emit headerDataChanged(Qt::Horizontal, 0, 0);
}

void HistoryQueryResultsModel::setLengthHeader(const QString &lengthHeader)
{
    if (LengthHeader == lengthHeader)
        return;

    LengthHeader = lengthHeader;
    emit headerDataChanged(Qt::Horizontal, 3, 3);
}

void HistoryQueryResultsModel::setResults(const QVector<HistoryQueryResult> &results)
{
    beginResetModel();
    Results = results;
    endResetModel();
}

void HistoryQueryResultsModel::addEntry(const QDate &date, const Talkable &talkable, const QString &title)
{
    auto i = 0;
    auto firstNotEarlier =
        std::find_if(std::begin(Results), std::end(Results), [&date, &i](const HistoryQueryResult &hqr) {
            i++;
            return hqr.date() >= date;
        });
    if (firstNotEarlier != std::end(Results) && firstNotEarlier->date() == date)
    {
        firstNotEarlier->setCount(firstNotEarlier->count() + 1);
        emit dataChanged(index(i - 1), index(i - 1));
        return;
    }

    auto newItem = HistoryQueryResult{};
    newItem.setCount(1);
    newItem.setDate(date);
    newItem.setTalkable(talkable);
    newItem.setTitle(title);

    beginInsertRows(QModelIndex{}, i, i);
    Results.insert(i, newItem);
    endInsertRows();
}
