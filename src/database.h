/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QSqlQuery>

class Database : public QObject
{
    Q_OBJECT

public:
    static Database &instance()
    {
        static Database _instance;
        return _instance;
    }
    bool execute(QSqlQuery &query);
    bool execute(const QString &query);
    QString defaultGroup();
    Q_INVOKABLE void addChannel(const QString &url, const QString &groupName = QString());
    Q_INVOKABLE void importChannels(const QString &path);
    Q_INVOKABLE void exportChannels(const QString &path);
    Q_INVOKABLE void addChannelGroup(const QString &name, const QString &description, const int isDefault = 0);
    Q_INVOKABLE void removeChannelGroup(const QString &name);
    Q_INVOKABLE void setDefaultGroup(const QString &name);
    Q_INVOKABLE void editChannel(const QString &url, const QString &displayName, const QString &groupName);

Q_SIGNALS:
    void channelAdded(const QString &url);
    void channelDetailsUpdated(const QString &url, const QString &displayName, const QString &description);
    void channelGroupsUpdated();
    void channelGroupRemoved(const QString &groupName);

private:
    bool channelGroupExists(const QString &name);
    void clearChannelGroup(const QString &name);

    Database();
    int version();
    bool migrateTo(const int targetVersion);
    bool migrateTo1();
    bool migrateTo2();
    void cleanup();
    bool channelExists(const QString &url);
};
