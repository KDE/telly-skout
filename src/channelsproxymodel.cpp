// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "channelsproxymodel.h"

#include "channel.h"
#include "database.h"

ChannelsProxyModel::ChannelsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_onlyFavorites(false)
    , m_group("")
{
    connect(&Database::instance(), &Database::channelDetailsUpdated, this, [this]() {
        invalidateFilter();
    });
}

ChannelsProxyModel::~ChannelsProxyModel()
{
}

bool ChannelsProxyModel::onlyFavorites() const
{
    return m_onlyFavorites;
}

void ChannelsProxyModel::setOnlyFavorites(const bool &onlyFavorites)
{
    if (m_onlyFavorites != onlyFavorites) {
        m_onlyFavorites = onlyFavorites;
        invalidateFilter();
        Q_EMIT onlyFavoritesChanged();
    }
}

const QString &ChannelsProxyModel::group() const
{
    return m_group.value();
}

void ChannelsProxyModel::setGroup(const QString &group)
{
    if (m_group.value() != group) {
        m_group = GroupId(group);
        invalidateFilter();
        Q_EMIT groupChanged();
    }
}

bool ChannelsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto idx = sourceModel()->index(source_row, 0, source_parent);

    if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)) {
        return false;
    }

    // no filter
    if (!m_onlyFavorites && m_group.value().isEmpty()) {
        return true;
    }

    // at least one filter
    auto channel = idx.data(0).value<Channel *>();

    const bool onlyFavoritesMatches = !m_onlyFavorites || channel->favorite();
    const bool groupMatches = m_group.value().isEmpty() || channel->groups().contains(m_group.value());

    return onlyFavoritesMatches && groupMatches;
}
