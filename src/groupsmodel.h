// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QAbstractListModel>

#include "groupfactory.h"

class Group;

class GroupsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GroupsModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;

private:
    void loadGroup(int index) const;

    mutable QVector<Group *> m_groups;
    GroupFactory m_groupFactory;
};
