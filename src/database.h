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
    Q_INVOKABLE void addChannel(const QString &id, const QString &name, const QString &url, bool favorite = false);

Q_SIGNALS:
    void channelAdded(const QString &url);
    void channelDetailsUpdated(const QString &url, const QString &name, bool favorite);

private:
    Database();
    int version();
    bool createTables();
    void cleanup();
    bool channelExists(const QString &url);
};
