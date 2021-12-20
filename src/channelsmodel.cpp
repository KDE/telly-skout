/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "channelsmodel.h"

#include "channel.h"
#include "database.h"
#include "fetcher.h"

#include <QDebug>
#include <QSqlQuery>

ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // TODO: does not work (channels are sorted by name)
    // connect(&Database::instance(), &Database::channelAdded, this, [this]() {
    //    beginInsertRows(QModelIndex(), rowCount(QModelIndex()) - 1, rowCount(QModelIndex()) - 1);
    //    endInsertRows();
    //});
    connect(&Fetcher::instance(), &Fetcher::countryUpdated, this, [this](const CountryId &id) {
        Q_UNUSED(id)
        beginResetModel();
        qDeleteAll(m_channels);
        m_channels.clear();
        m_channelFactory.load(false);
        m_channelFactory.load(true);
        endResetModel();
    });

    connect(&Fetcher::instance(), &Fetcher::channelDetailsUpdated, this, [this](const ChannelId &id, const QString &image) {
        for (int i = 0; i < m_channels.length(); i++) {
            if (m_channels[i]->id() == id.value()) {
                m_channels[i]->setImage(image);
                // TODO: update channelFactory?
                Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                break;
            }
        }
    });

    connect(&Database::instance(), &Database::channelDetailsUpdated, [this](const ChannelId &id, bool favorite) {
        for (int i = 0; i < m_channels.length(); i++) {
            if (m_channels[i]->id() == id.value()) {
                m_channels[i]->setFavorite(favorite);
                // TODO: update channelFactory?
                Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                break;
            }
        }
    });
}

bool ChannelsModel::onlyFavorites() const
{
    return m_onlyFavorites;
}

void ChannelsModel::setOnlyFavorites(bool onlyFavorites)
{
    m_onlyFavorites = onlyFavorites;
}

QHash<int, QByteArray> ChannelsModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[0] = "channel";
    return roleNames;
}

int ChannelsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_channelFactory.count(m_onlyFavorites);
}

QVariant ChannelsModel::data(const QModelIndex &index, int role) const
{
    if (role != 0) {
        return QVariant();
    }
    if (m_channels.length() <= index.row()) {
        loadChannel(index.row());
    }
    return QVariant::fromValue(m_channels[index.row()]);
}

void ChannelsModel::loadChannel(int index) const
{
    m_channels += m_channelFactory.create(m_onlyFavorites, index);
}

void ChannelsModel::setFavorite(const QString &channelId, bool favorite)
{
    if (favorite) {
        Database::instance().addFavorite(ChannelId(channelId));
    } else {
        Database::instance().removeFavorite(ChannelId(channelId));
    }
}

void ChannelsModel::move(int from, int to)
{
    const int destination = to > from ? to + 1 : to;

    beginMoveRows(QModelIndex(), from, from, QModelIndex(), destination);
    m_channels.move(from, to);
    // rebuild favorites
    // TODO: smarter solution?
    for (auto &&channel : qAsConst(m_channels)) {
        Database::instance().removeFavorite(ChannelId(channel->id()), false);
    }
    for (auto &&channel : qAsConst(m_channels)) {
        Database::instance().addFavorite(ChannelId(channel->id()), false);
    }
    endMoveRows();
}
