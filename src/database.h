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

    void addFavorite(const ChannelId &channelId, bool emitSignal = true);
    void removeFavorite(const ChannelId &channelId, bool emitSignal = true);
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

private:
    Database();
    ~Database();
    int version();
    bool createTables();
    void cleanup();

    QSqlQuery *m_addCountryQuery;
    QSqlQuery *m_countryCountQuery;
    QSqlQuery *m_countryExistsQuery;
    QSqlQuery *m_countriesQuery;
    QSqlQuery *m_countriesPerChannelQuery;

    QSqlQuery *m_addCountryChannelQuery;

    QSqlQuery *m_addChannelQuery;
    QSqlQuery *m_channelCountQuery;
    QSqlQuery *m_channelExistsQuery;
    QSqlQuery *m_channelsQuery;
    QSqlQuery *m_channelQuery;

    QSqlQuery *m_favoriteCountQuery;
    QSqlQuery *m_favoritesQuery;
    QSqlQuery *m_isFavoriteQuery;

    QSqlQuery *m_addProgramQuery;
    QSqlQuery *m_updateProgramDescriptionQuery;
    QSqlQuery *m_programExistsQuery;
    QSqlQuery *m_programCountQuery;
    QSqlQuery *m_programsQuery;
    QSqlQuery *m_programsPerChannelQuery;
};
