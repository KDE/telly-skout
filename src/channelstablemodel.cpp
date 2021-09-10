/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <algorithm>

#include <QBrush>
#include <QDebug>
#include <QModelIndex>
#include <QSqlQuery>
#include <QUrl>
#include <QVariant>

#include "channel.h"
#include "channelstablemodel.h"
#include "database.h"
#include "fetcher.h"
#include "program.h"

ChannelsTableModel::ChannelsTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(&Fetcher::instance(), &Fetcher::finishedFetchingFavorites, this, [this]() {
        // cleanup
        foreach (Channel *channel, m_channels) {
            // TODO: store programs in channel and delete them as well (Channel as parent?)
            delete channel;
        }
        m_channels.clear();
        m_programs.clear();

        // reload
        load();
        // TODO: why does dataChanged() not work?
        Q_EMIT dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, columnCount() - 1));
    });

    // preload
    load();
}

QHash<int, QByteArray> ChannelsTableModel::roleNames() const
{
    return {{Qt::DisplayRole, "program"}, {Qt::UserRole, "isFirst"}};
}

int ChannelsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return numRows;
}

int ChannelsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT() FROM Channels WHERE favorite=1;"));
    Database::instance().execute(query);
    if (!query.next()) {
        qWarning() << "Failed to query channel count";
    }
    return query.value(0).toInt();
}

QVariant ChannelsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role) {
        switch (orientation) {
        case Qt::Orientation::Horizontal:
            if (m_channels.length() <= section) {
                loadChannel(section);
            }
            return m_channels[section]->name();
        case Qt::Orientation::Vertical:
            const int hours = section / 60;
            const int minutes = section % 60;
            // only for full hour
            if (minutes == 0) {
                return QString("%1:%2").arg(hours, 2, 10, QLatin1Char('0')).arg(minutes, 2, 10, QLatin1Char('0'));
            } else {
                return QString("");
            }
        }
    }
    return QVariant();
}

QVariant ChannelsTableModel::data(const QModelIndex &index, int role) const
{
    if (m_channels.length() <= index.column()) {
        loadChannel(index.column());
    }
    Program *program = m_programs[index.column()].at(index.row());
    if (!program) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        return QVariant::fromValue(program);
    }
    case Qt::UserRole: {
        // check if progam starts now
        // offset for today (00:00) [UTC] + row [min] * 60 => second [since 1970] when program is running
        QDateTime utcTimeToday(QDate::currentDate(), QTime(), Qt::LocalTime);
        const qint64 offsetTimeToday = utcTimeToday.toSecsSinceEpoch();
        const qint64 second = offsetTimeToday + (index.row() * 60);
        return second == program->start().toSecsSinceEpoch();
    }
    }

    return QVariant();
}

void ChannelsTableModel::load() const
{
    for (int column = 0; column < columnCount(); ++column) {
        loadChannel(column);
    }
}

void ChannelsTableModel::loadChannel(int index) const
{
    Channel *channel = new Channel(index, true);
    m_channels += channel;

    // load program
    if (m_programs.size() <= index) { // should always be true but just in case...
        m_programs.insert(index, std::array<Program *, numRows>());
    }
    // offset for today (00:00) [UTC] + row [min] * 60 => second [since 1970] when program is running
    QDateTime utcTimeToday(QDate::currentDate(), QTime(), Qt::LocalTime);
    const qint64 offsetTimeToday = utcTimeToday.toSecsSinceEpoch();

    QSqlQuery programQuery;
    // only programs which run today
    programQuery.prepare(QStringLiteral("SELECT * FROM Programs WHERE channel=:channel AND start <= :end AND stop >= :begin;"));
    programQuery.bindValue(QStringLiteral(":channel"), channel->url());
    programQuery.bindValue(QStringLiteral(":end"), offsetTimeToday + (24 * 60 * 60));
    programQuery.bindValue(QStringLiteral(":begin"), offsetTimeToday);
    Database::instance().execute(programQuery);
    while (programQuery.next()) {
        const int start = programQuery.value(QStringLiteral("start")).toInt();
        const int stop = programQuery.value(QStringLiteral("stop")).toInt();
        Program *program = new Program(channel, QDateTime().fromSecsSinceEpoch(start)); // TODO fix memleak

        // remember for all rows (1 per minute) when this program is running
        const int firstRow = std::max((start - offsetTimeToday) / 60, static_cast<qint64>(0));
        const int lastRow = std::min((stop - offsetTimeToday) / 60, static_cast<qint64>(numRows));
        for (int row = firstRow; row < lastRow; ++row) {
            m_programs[index].at(row) = program;
        }
    }
}

void ChannelsTableModel::refreshAll()
{
    for (auto &channel : m_channels) {
        channel->refresh();
    }
}
