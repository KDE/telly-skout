// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QSortFilterProxyModel>

#include "types.h"

#include <QQmlEngine>

class ChannelsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool onlyFavorites READ onlyFavorites WRITE setOnlyFavorites NOTIFY onlyFavoritesChanged)
    Q_PROPERTY(GroupId group READ group WRITE setGroup NOTIFY groupChanged)

public:
    explicit ChannelsProxyModel(QObject *parent = nullptr);
    ~ChannelsProxyModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool onlyFavorites() const;
    void setOnlyFavorites(const bool &onlyFavorites);

    const GroupId &group() const;
    void setGroup(const GroupId &group);

Q_SIGNALS:
    void onlyFavoritesChanged();
    void groupChanged();

private:
    bool m_onlyFavorites;
    GroupId m_group;
};
