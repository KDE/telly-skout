// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "channeldata.h"
#include "groupdata.h"
#include "programdata.h"
#include "types.h"

#include <QList>
#include <QMap>
#include <QMultiMap>
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

    bool execute(QSqlQuery &query) const;
    bool execute(const QString &query) const;

    void addGroup(const GroupData &data);
    void addGroups(const QVector<GroupData> &groups);
    size_t groupCount() const;
    bool groupExists(const GroupId &id) const;
    QVector<GroupData> groups() const;
    QVector<GroupData> groups(const ChannelId &channelId) const;

    void addChannel(const ChannelData &data, const GroupId &group);
    void addChannels(const QList<ChannelData> &channels, const GroupId &group);
    size_t channelCount() const;
    bool channelExists(const ChannelId &id) const;
    QVector<ChannelData> channels(bool onlyFavorites) const;
    ChannelData channel(const ChannelId &channelId) const;

    void addFavorite(const ChannelId &channelId);
    void removeFavorite(const ChannelId &channelId);
    void sortFavorites(const QVector<ChannelId> &newOrder); // newOrder must contain same channel IDs as existing favorites
    void clearFavorites();
    size_t favoriteCount() const;
    QVector<ChannelId> favorites() const;
    bool isFavorite(const ChannelId &channelId) const;

    void addProgram(const ProgramData &data);
    void updateProgramDescription(const ProgramId &id, const QString &description);
    void addPrograms(const QVector<ProgramData> &programs);
    bool programExists(const ChannelId &channelId, const QDateTime &lastTime) const;
    size_t programCount(const ChannelId &channelId) const;
    QMap<ChannelId, QVector<ProgramData>> programs() const;
    QVector<ProgramData> programs(const ChannelId &channelId) const;
    ProgramData program(const ProgramId &programId) const;

Q_SIGNALS:
    void groupAdded(const GroupId &id);
    void channelAdded(const ChannelId &id);
    void channelDetailsUpdated(const ChannelId &id, bool favorite);
    void favoritesUpdated();

private:
    Database();
    ~Database() = default;

    int version() const;
    int fetcher() const;
    bool createTables();
    bool dropTables();
    void cleanup();
    QMultiMap<ProgramId, QString> programCategories() const;

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
    std::unique_ptr<QSqlQuery> m_programQuery;
};
