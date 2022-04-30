// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include "channeldata.h"
#include "countrydata.h"
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

    void addCountry(const CountryId &id, const QString &name, const QString &url);
    size_t countryCount();
    bool countryExists(const CountryId &id);
    QVector<CountryData> countries();
    QVector<CountryData> countries(const ChannelId &channelId);

    void addChannel(const ChannelData &data, const CountryId &country);
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
    void countryAdded(const CountryId &id);
    void channelAdded(const ChannelId &id);
    void channelDetailsUpdated(const ChannelId &id, bool favorite);
    void favoritesUpdated();

private:
    Database();
    ~Database();
    int version();
    bool createTables();
    void cleanup();

    std::unique_ptr<QSqlQuery> m_addCountryQuery;
    std::unique_ptr<QSqlQuery> m_countryCountQuery;
    std::unique_ptr<QSqlQuery> m_countryExistsQuery;
    std::unique_ptr<QSqlQuery> m_countriesQuery;
    std::unique_ptr<QSqlQuery> m_countriesPerChannelQuery;

    std::unique_ptr<QSqlQuery> m_addCountryChannelQuery;

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

    std::unique_ptr<QSqlQuery> m_addProgramQuery;
    std::unique_ptr<QSqlQuery> m_updateProgramDescriptionQuery;
    std::unique_ptr<QSqlQuery> m_programExistsQuery;
    std::unique_ptr<QSqlQuery> m_programCountQuery;
    std::unique_ptr<QSqlQuery> m_programsQuery;
    std::unique_ptr<QSqlQuery> m_programsPerChannelQuery;
};
