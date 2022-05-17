// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QSortFilterProxyModel>

#include "types.h"

class ChannelsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool onlyFavorites READ onlyFavorites WRITE setOnlyFavorites NOTIFY onlyFavoritesChanged)
    Q_PROPERTY(QString group READ group WRITE setGroup NOTIFY groupChanged)

public:
    explicit ChannelsProxyModel(QObject *parent = nullptr);
    ~ChannelsProxyModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool onlyFavorites() const;
    void setOnlyFavorites(const bool &onlyFavorites);

    const QString &group() const;
    void setGroup(const QString &group);

Q_SIGNALS:
    void onlyFavoritesChanged();
    void groupChanged();

private:
    bool m_onlyFavorites;
    GroupId m_group;
};
