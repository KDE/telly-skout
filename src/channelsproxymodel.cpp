/*
 * SPDX-FileCopyrightText: 2021 Dimitris Kardarakos <dimkard@posteo.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "channelsproxymodel.h"
#include "channel.h"
#include "channelsmodel.h"
#include "database.h"

ChannelsProxyModel::ChannelsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_group_name{}
{
    connect(&Database::instance(), &Database::channelDetailsUpdated, [this]() {
        invalidateFilter();
    });
}

ChannelsProxyModel::~ChannelsProxyModel()
{
}

QString ChannelsProxyModel::groupName() const
{
    return m_group_name;
}

void ChannelsProxyModel::setGroupName(const QString &name)
{
    if (m_group_name != name) {
        m_group_name = name;
        invalidateFilter();
        Q_EMIT groupNameChanged();
    }
}

QString ChannelsProxyModel::country() const
{
    return m_country;
}

void ChannelsProxyModel::setCountry(const QString &country)
{
    if (m_country != country) {
        m_country = country;
        invalidateFilter();
        Q_EMIT countryChanged();
    }
}

bool ChannelsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto idx = sourceModel()->index(source_row, 0, source_parent);

    if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)) {
        return false;
    }

    // no filter
    if (m_group_name.isEmpty() && m_country.isEmpty()) {
        return true;
    }

    // at least one filter
    auto channel = idx.data(0).value<Channel *>();

    const bool groupNameMatches = m_group_name.isEmpty() || channel->favorite();
    const bool countryMatches = m_country.isEmpty() || channel->country() == m_country;

    return groupNameMatches && countryMatches;
}
