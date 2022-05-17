// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include "TellySkoutSettings.h"
#include "channeldata.h"
#include "groupdata.h"
#include "programdata.h"
#include "types.h"

#include <QMap>
#include <QSqlQuery>
#include <QString>
#include <QVector>

#include <memory>

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

    void addGroup(const GroupId &id, const QString &name, const QString &url);
    size_t groupCount();
    bool groupExists(const GroupId &id);
    QVector<GroupData> groups();
    QVector<GroupData> groups(const ChannelId &channelId);

    void addChannel(const ChannelData &data, const GroupId &group);
    size_t channelCount();
    bool channelExists(const ChannelId &id);
    QVector<ChannelData> channels(bool onlyFavorites);
    ChannelData channel(const ChannelId &channelId);

    void addFavorite(const ChannelId &channelId);
    void removeFavorite(const ChannelId &channelId);
    void sortFavorites(const QVector<ChannelId> &newOrder); // newOrder must contain same channel IDs as existing favorites
    void clearFavorites();
    size_t favoriteCount();
    QVector<ChannelId> favorites();
    bool isFavorite(const ChannelId &channelId);

    void addProgram(const ProgramData &data);
    void updateProgramDescription(const ProgramId &id, const QString &description);
    void addPrograms(const QVector<ProgramData> &programs);
    bool programExists(const ChannelId &channelId, qint64 lastTime);
    size_t programCount(const ChannelId &channelId);
    QMap<ChannelId, QVector<ProgramData>> programs();
    QVector<ProgramData> programs(const ChannelId &channelId);

Q_SIGNALS:
    void groupAdded(const GroupId &id);
    void channelAdded(const ChannelId &id);
    void channelDetailsUpdated(const ChannelId &id, bool favorite);
    void favoritesUpdated();

private:
    Database();
    ~Database();
    int version();
    int fetcher();
    bool createTables();
    bool dropTables();
    void cleanup();

    const TellySkoutSettings m_settings;

    std::unique_ptr<QSqlQuery> m_addGroupQuery;
    std::unique_ptr<QSqlQuery> m_groupCountQuery;
    std::unique_ptr<QSqlQuery> m_groupExistsQuery;
    std::unique_ptr<QSqlQuery> m_groupsQuery;
    std::unique_ptr<QSqlQuery> m_groupsPerChannelQuery;

    std::unique_ptr<QSqlQuery> m_addGroupChannelQuery;

    std::unique_ptr<QSqlQuery> m_addChannelQuery;
    std::unique_ptr<QSqlQuery> m_channelCountQuery;
    std::unique_ptr<QSqlQuery> m_channelExistsQuery;
    std::unique_ptr<QSqlQuery> m_channelsQuery;
    std::unique_ptr<QSqlQuery> m_channelQuery;

    std::unique_ptr<QSqlQuery> m_addFavoriteQuery;
    std::unique_ptr<QSqlQuery> m_clearFavoritesQuery;
    std::unique_ptr<QSqlQuery> m_favoriteCountQuery;
    std::unique_ptr<QSqlQuery> m_favoritesQuery;
    std::unique_ptr<QSqlQuery> m_isFavoriteQuery;

    std::unique_ptr<QSqlQuery> m_addProgramCategoryQuery;
    std::unique_ptr<QSqlQuery> m_programCategoriesQuery;

    std::unique_ptr<QSqlQuery> m_addProgramQuery;
    std::unique_ptr<QSqlQuery> m_updateProgramDescriptionQuery;
    std::unique_ptr<QSqlQuery> m_programExistsQuery;
    std::unique_ptr<QSqlQuery> m_programCountQuery;
    std::unique_ptr<QSqlQuery> m_programsQuery;
    std::unique_ptr<QSqlQuery> m_programsPerChannelQuery;
};
