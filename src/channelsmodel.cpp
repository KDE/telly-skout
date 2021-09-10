/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QDebug>
#include <QModelIndex>
#include <QSqlQuery>
#include <QUrl>
#include <QVariant>

#include "channelsmodel.h"
#include "database.h"
#include "fetcher.h"

ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractListModel(parent)
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

QHash<int, QByteArray> ChannelsModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[0] = "channel";
    return roleNames;
}

int ChannelsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT() FROM Channels;"));
    Database::instance().execute(query);
    if (!query.next()) {
        qWarning() << "Failed to query channel count";
    }
    return query.value(0).toInt();
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
    m_channels += new Channel(index);
}

void ChannelsModel::setFavorite(const QString &channel, bool favorite)
{
    for (int i = 0; i < m_channels.length(); i++) {
        if (m_channels[i]->url() == channel) {
            m_channels[i]->setAsFavorite(favorite);
        }
    }
}

void ChannelsModel::refreshAll()
{
    for (auto &channel : m_channels) {
        channel->refresh();
    }
}
