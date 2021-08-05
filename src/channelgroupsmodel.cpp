/*
 * SPDX-FileCopyrightText: 2021 Dimitris Kardarakos <dimkard@posteo.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "channelgroupsmodel.h"
#include "database.h"
#include <QSqlQuery>

ChannelGroupsModel::ChannelGroupsModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_channel_groups{}
{
    loadFromDatabase();

    connect(&Database::instance(), &Database::channelGroupsUpdated, [this]() {
        loadFromDatabase();
    });

    connect(&Database::instance(), &Database::channelGroupRemoved, [this]() {
        loadFromDatabase();
    });
}

QHash<int, QByteArray> ChannelGroupsModel::roleNames() const
{
    return {{GroupName, "name"}, {GroupDescription, "description"}, {IsDefault, "isDefault"}};
}

int ChannelGroupsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_channel_groups.count();
}

QVariant ChannelGroupsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    auto row = index.row();

    switch (role) {
    case GroupName: {
        return m_channel_groups.at(row).name;
    }
    case GroupDescription: {
        return m_channel_groups.at(row).description;
    }
    case IsDefault:
        return m_channel_groups.at(row).isDefault;
    }

    return QVariant();
}

void ChannelGroupsModel::loadFromDatabase()
{
    beginResetModel();

    m_channel_groups = {};
    QSqlQuery q;
    q.prepare(QStringLiteral("SELECT * FROM ChannelGroups;"));
    Database::instance().execute(q);
    while (q.next()) {
        ChannelGroup group{};
        group.name = q.value(QStringLiteral("name")).toString();
        group.description = q.value(QStringLiteral("description")).toString();
        group.isDefault = (q.value(QStringLiteral("defaultGroup")).toInt() == 1);
        m_channel_groups << group;
    }

    endResetModel();
}
