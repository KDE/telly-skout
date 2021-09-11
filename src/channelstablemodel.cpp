/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "channelstablemodel.h"

#include "channel.h"
#include "database.h"
#include "fetcher.h"
#include "program.h"

#include <QDebug>
#include <QSqlQuery>

ChannelsTableModel::ChannelsTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(&Fetcher::instance(), &Fetcher::finishedFetchingFavorites, this, [this]() {
        // cleanup
        foreach (Channel *channel, m_channels) {
            delete channel;
        }
        m_channels.clear();
        m_programs.clear();
        m_isFirst.clear();

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
    return {{Qt::DisplayRole, "program"}, {Qt::UserRole, "metaData"}};
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
        QVariantMap metaData;
        metaData.insert("isFirst", false);
        metaData.insert("isRunning", false);
        if (index.column() < m_isFirst.size() && static_cast<size_t>(index.row()) < m_isFirst[index.column()].size()) {
            metaData["isFirst"] = m_isFirst[index.column()].at(index.row());
        }
        const int rowNow = row(QDateTime::currentDateTime().toSecsSinceEpoch());
        const Program *programNow = m_programs[index.column()].at(rowNow);
        metaData["isRunning"] = (program->id() == programNow->id()) && (index.row() <= rowNow);
        return QVariant::fromValue(metaData);
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

    // programs
    if (m_programs.size() <= index) { // should always be true but just in case...
        m_programs.insert(index, std::array<Program *, numRows>());
    }
    if (m_isFirst.size() <= index) { // should always be true but just in case...
        m_isFirst.insert(index, std::array<bool, numRows>());
    }

    const auto programs = channel->programs();
    for (const auto program : programs) {
        // only programs which run today
        const int start = program->start().toSecsSinceEpoch();
        const int stop = program->stop().toSecsSinceEpoch();
        if ((start <= timestampToday() + (24 * 60 * 60)) && (stop >= timestampToday())) {
            // remember for all rows (1 per minute) when this program is running
            const int firstRow = std::max(row(start), 0);
            const int lastRow = std::min(row(stop), rowCount());
            for (int row = firstRow; row < lastRow; ++row) {
                m_programs[index].at(row) = program;

                // check if progam starts now (is first)
                m_isFirst[index].at(row) = timestamp(row) == start;
            }
        }
    }
}

qint64 ChannelsTableModel::timestampToday() const
{
    return QDateTime(QDate::currentDate(), QTime(), Qt::LocalTime).toSecsSinceEpoch();
}

qint64 ChannelsTableModel::timestamp(int row) const
{
    return timestampToday() + (row * 60);
}

int ChannelsTableModel::row(qint64 timestamp) const
{
    return (timestamp - timestampToday()) / 60;
}

void ChannelsTableModel::refreshAll()
{
    for (auto &channel : m_channels) {
        channel->refresh();
    }
}
