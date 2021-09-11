/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

class QSqlQuery;

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
    Q_INVOKABLE void addCountry(const QString &id, const QString &name, const QString &url);
    Q_INVOKABLE void
    addChannel(const QString &id, const QString &name, const QString &url, const QString &country, const QString &image, bool favorite = false);

Q_SIGNALS:
    void countryAdded(const QString &url);
    void channelAdded(const QString &url);
    void countryDetailsUpdated(const QString &id);
    void channelDetailsUpdated(const QString &id, bool favorite);

private:
    Database();
    int version();
    bool createTables();
    void cleanup();
    bool countryExists(const QString &url);
    bool channelExists(const QString &url);
};
