/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

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
    connect(&Database::instance(), &Database::channelAdded, this, [this]() {
        beginInsertRows(QModelIndex(), rowCount(QModelIndex()) - 1, rowCount(QModelIndex()) - 1);
        endInsertRows();
    });

    connect(&Fetcher::instance(), &Fetcher::channelDetailsUpdated, this, [this](const QString &id, const QString &image) {
        for (int i = 0; i < m_channels.length(); i++) {
            if (m_channels[i]->id() == id) {
                m_channels[i]->setImage(image);
                Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                break;
            }
        }
    });

    connect(&Database::instance(), &Database::channelDetailsUpdated, [this](const QString &id, bool favorite) {
        for (int i = 0; i < m_channels.length(); i++) {
            if (m_channels[i]->id() == id) {
                m_channels[i]->setFavorite(favorite);
                Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                break;
            }
        }
    });
}

QHash<int, QByteArray> ChannelsTableModel::roleNames() const
{
    return {{Qt::DisplayRole, "program"}};
}

int ChannelsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 24 * 60; // one per minute per day
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
    switch (role) {
    case Qt::DisplayRole: {
        switch (orientation) {
        case Qt::Orientation::Horizontal:
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
        break;
    }
    case Qt::BackgroundRole: {
        // do not show empty header cells
        // TODO: doesn't work
        if ((Qt::Orientation::Vertical == orientation) && (section % 60) != 0) {
            return QVariant(QColor(Qt::transparent));
        }
    }
    }
    return QVariant();
}

QVariant ChannelsTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (m_channels.length() <= index.column()) {
            loadChannel(index.column());
        }
        Channel *channel = m_channels[index.column()];

        // get correct program for row
        // offset for today (00:00) [UTC] + row [min] * 60 => second [since 1970] when program is running
        // start <= second < end
        QDateTime utcTimeToday(QDate::currentDate(), QTime(), Qt::LocalTime);
        const qint64 offsetTimeToday = utcTimeToday.toSecsSinceEpoch();
        const qint64 second = offsetTimeToday + (index.row() * 60);

        if (m_programs.size() <= index.column()) {
            m_programs.insert(index.column(), QVector<Program *>());
        }
        if (m_programs[index.column()].length() <= index.row()) {
            m_programs[index.column()].push_back(new Program(channel, second));
        }
        Program *program = m_programs[index.column()].at(index.row());
        return QVariant::fromValue(program);
    }

    return QVariant();
}

void ChannelsTableModel::loadChannel(int index) const
{
    m_channels += new Channel(index, true);
}

void ChannelsTableModel::setChannelAsFavorite(const QString &url)
{
    for (int i = 0; i < m_channels.length(); i++) {
        if (m_channels[i]->url() == url) {
            m_channels[i]->setAsFavorite();
        }
    }
}

void ChannelsTableModel::refreshAll()
{
    for (auto &channel : m_channels) {
        channel->refresh();
    }
}
