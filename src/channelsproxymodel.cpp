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

void ChannelsProxyModel::setGroupName(const QString &name)
{
    if (m_group_name != name) {
        m_group_name = name;
        invalidateFilter();
        Q_EMIT groupNameChanged();
    }
}

QString ChannelsProxyModel::groupName() const
{
    return m_group_name;
}

bool ChannelsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto idx = sourceModel()->index(source_row, 0, source_parent);

    if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)) {
        return false;
    }

    if (m_group_name.isEmpty()) {
        return true;
    }

    auto channel = idx.data(0).value<Channel *>();

    if (channel->groupName() == m_group_name) {
        return true;
    }

    return false;
}
