/*
 * SPDX-FileCopyrightText: 2021 Dimitris Kardarakos <dimkard@posteo.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CHANNEL_GROUPS_MODEL_H
#define CHANNEL_GROUPS_MODEL_H

#include <QAbstractListModel>

/**
 * @brief Data structure to store a single channel group program
 *
 */
struct ChannelGroup {
    QString name;
    QString description;
    bool isDefault;
};

/**
 * @brief Model that provides the channel groups
 *
 */
class ChannelGroupsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ChannelGroupsModel(QObject *parent = nullptr);

    enum RoleNames { GroupName = Qt::UserRole + 1, GroupDescription, IsDefault };

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;

private:
    void loadFromDatabase();
    QVector<ChannelGroup> m_channel_groups;
};
#endif
