// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "groupsmodel.h"

#include "database.h"
#include "fetcher.h"
#include "group.h"
#include "types.h"

#include <QDebug>

#include <limits>

GroupsModel::GroupsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&Database::instance(), &Database::groupAdded, this, [this]() {
        m_groupFactory.load();
        beginInsertRows(QModelIndex(), rowCount(QModelIndex()) - 1, rowCount(QModelIndex()) - 1);
        endInsertRows();
    });
}

QHash<int, QByteArray> GroupsModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[0] = "group";
    return roleNames;
}

int GroupsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    Q_ASSERT(m_groupFactory.count() <= std::numeric_limits<int>::max());
    return static_cast<int>(m_groupFactory.count());
}

QVariant GroupsModel::data(const QModelIndex &index, int role) const
{
    if (role != 0) {
        return QVariant();
    }
    if (m_groups.length() <= index.row()) {
        loadGroup(index.row());
    }
    return QVariant::fromValue(m_groups[index.row()]);
}

void GroupsModel::loadGroup(int index) const
{
    m_groups += m_groupFactory.create(index);
}
