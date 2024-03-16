// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QAbstractListModel>

#include "channelfactory.h"

#include <QUrl>

class Channel;

class ChannelsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool onlyFavorites READ onlyFavorites WRITE setOnlyFavorites)

public:
    explicit ChannelsModel(QObject *parent = nullptr);
    ~ChannelsModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    Q_INVOKABLE void setFavorite(const ChannelId &channelId, bool favorite);
    Q_INVOKABLE void move(int from, int to);
    Q_INVOKABLE void save();

    bool onlyFavorites() const;
    void setOnlyFavorites(bool onlyFavorites);

private:
    void loadChannel(int index) const;

    mutable QVector<Channel *> m_channels;
    bool m_onlyFavorites;
    ChannelFactory m_channelFactory;
};
