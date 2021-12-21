#include "database.h"

#include "TellySkoutSettings.h"
#include "fetcher.h"

#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStandardPaths>
#include <QUrl>

#define TRUE_OR_RETURN(x)                                                                                                                                      \
    if (!x)                                                                                                                                                    \
        return false;

Database::Database()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    const QString databasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir(databasePath).mkpath(databasePath);
    db.setDatabaseName(databasePath + QStringLiteral("/database.db3"));
    db.open();

    if (!createTables()) {
        qCritical() << "Failed to create database";
    }

    cleanup();

    // speed up database (especially for slow persistent memory like on the PinePhone)
    execute(QStringLiteral("PRAGMA synchronous = OFF;"));
    execute(QStringLiteral("PRAGMA journal_mode = WAL;")); // TODO: or MEMORY?
    execute(QStringLiteral("PRAGMA temp_store = MEMORY;"));
    execute(QStringLiteral("PRAGMA locking_mode = EXCLUSIVE;"));

    // prepare queries once (faster)
    m_addCountryQuery = new QSqlQuery(db);
    m_addCountryQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO Countries VALUES (:id, :name, :url);"));
    m_countryCountQuery = new QSqlQuery(db);
    m_countryCountQuery->prepare(QStringLiteral("SELECT COUNT() FROM Countries;"));
    m_countryExistsQuery = new QSqlQuery(db);
    m_countryExistsQuery->prepare(QStringLiteral("SELECT COUNT () FROM Countries WHERE id=:id;"));
    m_countriesQuery = new QSqlQuery(db);
    m_countriesQuery->prepare(QStringLiteral("SELECT * FROM Countries ORDER BY name COLLATE NOCASE;"));
    m_countriesPerChannelQuery = new QSqlQuery(db);
    m_countriesPerChannelQuery->prepare(
        QStringLiteral("SELECT * FROM Countries WHERE id=(SELECT country from CountryChannels WHERE channel=:channel) ORDER BY name COLLATE NOCASE;"));

    m_addCountryChannelQuery = new QSqlQuery(db);
    m_addCountryChannelQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO CountryChannels VALUES (:id, :country, :channel);"));

    m_addFavoriteQuery = new QSqlQuery(db);
    m_addFavoriteQuery->prepare(QStringLiteral("INSERT INTO Favorites VALUES ((SELECT COUNT() FROM Favorites) + 1, :channel);"));
    m_addChannelQuery = new QSqlQuery(db);
    m_addChannelQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO Channels VALUES (:id, :name, :url, :image);"));
    m_channelCountQuery = new QSqlQuery(db);
    m_channelCountQuery->prepare(QStringLiteral("SELECT COUNT() FROM Channels;"));
    m_channelExistsQuery = new QSqlQuery(db);
    m_channelExistsQuery->prepare(QStringLiteral("SELECT COUNT () FROM Channels WHERE id=:id;"));
    m_channelsQuery = new QSqlQuery(db);
    m_channelsQuery->prepare(QStringLiteral("SELECT * FROM Channels ORDER BY name COLLATE NOCASE;"));
    m_channelQuery = new QSqlQuery(db);
    m_channelQuery->prepare(QStringLiteral("SELECT * FROM Channels WHERE id=:channelId;"));

    m_removeFavoriteQuery = new QSqlQuery(db);
    m_removeFavoriteQuery->prepare(QStringLiteral("DELETE FROM Favorites WHERE channel=:channel;"));
    m_clearFavoritesQuery = new QSqlQuery(db);
    m_clearFavoritesQuery->prepare(QStringLiteral("DELETE FROM Favorites;"));
    m_favoriteCountQuery = new QSqlQuery(db);
    m_favoriteCountQuery->prepare(QStringLiteral("SELECT COUNT() FROM Favorites;"));
    m_favoritesQuery = new QSqlQuery(db);
    m_favoritesQuery->prepare(QStringLiteral("SELECT channel FROM Favorites ORDER BY id;"));
    m_isFavoriteQuery = new QSqlQuery(db);
    m_isFavoriteQuery->prepare(QStringLiteral("SELECT COUNT() FROM Favorites WHERE channel=:channel"));

    m_addProgramQuery = new QSqlQuery(db);
    m_addProgramQuery->prepare(
        QStringLiteral("INSERT OR IGNORE INTO Programs VALUES (:id, :url, :channel, :start, :stop, :title, :subtitle, :description, :category);"));
    m_updateProgramDescriptionQuery = new QSqlQuery(db);
    m_updateProgramDescriptionQuery->prepare(QStringLiteral("UPDATE Programs SET description=:description WHERE id=:id;"));
    m_programExistsQuery = new QSqlQuery(db);
    m_programExistsQuery->prepare(QStringLiteral("SELECT COUNT () FROM Programs WHERE channel=:channel AND stop>=:lastTime;"));
    m_programCountQuery = new QSqlQuery(db);
    m_programCountQuery->prepare(QStringLiteral("SELECT COUNT() FROM Programs WHERE channel=:channel;"));
    m_programsQuery = new QSqlQuery(db);
    m_programsQuery->prepare(QStringLiteral("SELECT * FROM Programs WHERE channel=:channel ORDER BY channel, start;"));
    m_programsPerChannelQuery = new QSqlQuery(db);
    m_programsPerChannelQuery->prepare(QStringLiteral("SELECT * FROM Programs WHERE channel=:channel ORDER BY start;"));
}

Database::~Database()
{
    delete m_addCountryQuery;
    delete m_addCountryChannelQuery;
    delete m_addChannelQuery;
    delete m_addProgramQuery;
    delete m_updateProgramDescriptionQuery;
    delete m_programExistsQuery;
}

bool Database::createTables()
{
    qDebug() << "Create DB tables";
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Countries (id TEXT UNIQUE, name TEXT, url TEXT);")));
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Channels (id TEXT UNIQUE, name TEXT, url TEXT, image TEXT);")));
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS CountryChannels (id TEXT UNIQUE, country TEXT, channel TEXT);")));
    TRUE_OR_RETURN(execute(
        QStringLiteral("CREATE TABLE IF NOT EXISTS Programs (id TEXT UNIQUE, url TEXT, channel TEXT, start INTEGER, stop INTEGER, title TEXT, subtitle TEXT, "
                       "description TEXT, category TEXT);")));
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Favorites (id INTEGER UNIQUE, channel TEXT UNIQUE);")));

    TRUE_OR_RETURN(execute(QStringLiteral("PRAGMA user_version = 1;")));
    return true;
}

bool Database::execute(const QString &query)
{
    QSqlQuery q;
    q.prepare(query);
    return execute(q);
}

bool Database::execute(QSqlQuery &query)
{
    if (!query.exec()) {
        qWarning() << "Failed to execute SQL Query";
        qWarning() << query.lastQuery();
        qWarning() << query.lastError();
        return false;
    }
    return true;
}

int Database::version()
{
    QSqlQuery query;
    query.prepare(QStringLiteral("PRAGMA user_version;"));
    execute(query);
    if (query.next()) {
        bool ok;
        int value = query.value(0).toInt(&ok);
        qDebug() << "Database version " << value;
        if (ok) {
            return value;
        }
    } else {
        qCritical() << "Failed to check database version";
    }
    return -1;
}

void Database::cleanup()
{
    const TellySkoutSettings settings;
    const unsigned int days = settings.deleteProgramAfter();

    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime = dateTime.addDays(-static_cast<qint64>(days));
    const qint64 sinceEpoch = dateTime.toSecsSinceEpoch();

    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM Programs WHERE stop < :sinceEpoch;"));
    query.bindValue(QStringLiteral(":sinceEpoch"), sinceEpoch);
    execute(query);
}

void Database::addCountry(const CountryId &id, const QString &name, const QString &url)
{
    if (!countryExists(id)) {
        qDebug() << "Add country" << name;
        m_addCountryQuery->bindValue(QStringLiteral(":id"), id.value());
        m_addCountryQuery->bindValue(QStringLiteral(":name"), name);
        m_addCountryQuery->bindValue(QStringLiteral(":url"), url);
        execute(*m_addCountryQuery);

        Q_EMIT countryAdded(id);
    }
}

size_t Database::countryCount()
{
    execute(*m_countryCountQuery);
    if (!m_countryCountQuery->next()) {
        qWarning() << "Failed to query country count";
        return 0;
    }
    return m_countryCountQuery->value(0).toInt();
}

bool Database::countryExists(const CountryId &id)
{
    m_countryExistsQuery->bindValue(QStringLiteral(":id"), id.value());
    execute(*m_countryExistsQuery);
    m_countryExistsQuery->next();

    return m_countryExistsQuery->value(0).toInt() > 0;
}

QVector<CountryData> Database::countries()
{
    QVector<CountryData> countries;

    execute(*m_countriesQuery);
    while (m_countriesQuery->next()) {
        CountryData data;
        data.m_id = CountryId(m_countriesQuery->value(QStringLiteral("id")).toString());
        data.m_name = m_countriesQuery->value(QStringLiteral("name")).toString();
        data.m_url = m_countriesQuery->value(QStringLiteral("url")).toString();
        countries.append(data);
    }
    return countries;
}

QVector<CountryData> Database::countries(const ChannelId &channelId)
{
    QVector<CountryData> countries;

    m_countriesPerChannelQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    execute(*m_countriesPerChannelQuery);
    while (m_countriesPerChannelQuery->next()) {
        CountryData data;
        data.m_id = CountryId(m_countriesPerChannelQuery->value(QStringLiteral("id")).toString());
        data.m_name = m_countriesPerChannelQuery->value(QStringLiteral("name")).toString();
        data.m_url = m_countriesPerChannelQuery->value(QStringLiteral("url")).toString();
        countries.append(data);
    }
    return countries;
}

void Database::addChannel(const ChannelData &data, const CountryId &country)
{
    if (!channelExists(data.m_id)) {
        qDebug() << "Add channel" << data.m_name;

        // store channel per country
        {
            m_addCountryChannelQuery->bindValue(QStringLiteral(":id"), country.value() + "_" + data.m_id.value());
            m_addCountryChannelQuery->bindValue(QStringLiteral(":country"), country.value());
            m_addCountryChannelQuery->bindValue(QStringLiteral(":channel"), data.m_id.value());
            execute(*m_addCountryChannelQuery);
        }

        // store channel
        {
            QUrl urlFromInput = QUrl::fromUserInput(data.m_url);
            m_addChannelQuery->bindValue(QStringLiteral(":id"), data.m_id.value());
            m_addChannelQuery->bindValue(QStringLiteral(":name"), data.m_name);
            m_addChannelQuery->bindValue(QStringLiteral(":url"), urlFromInput.toString());
            m_addChannelQuery->bindValue(QStringLiteral(":country"), country.value());
            m_addChannelQuery->bindValue(QStringLiteral(":image"), data.m_image);
            execute(*m_addChannelQuery);
            Q_EMIT channelAdded(data.m_id);
        }
    }
}

size_t Database::channelCount()
{
    execute(*m_channelCountQuery);
    if (!m_channelCountQuery->next()) {
        qWarning() << "Failed to query channel count";
        return 0;
    }
    return m_channelCountQuery->value(0).toInt();
}

bool Database::channelExists(const ChannelId &id)
{
    m_channelExistsQuery->bindValue(QStringLiteral(":id"), id.value());
    execute(*m_channelExistsQuery);
    m_channelExistsQuery->next();

    return m_channelExistsQuery->value(0).toInt() > 0;
}

QVector<ChannelData> Database::channels(bool onlyFavorites)
{
    QVector<ChannelData> channels;

    if (onlyFavorites) {
        const QVector<ChannelId> &favoriteIds = favorites();

        QSqlDatabase::database().transaction();
        for (int i = 0; i < favoriteIds.size(); ++i) {
            channels.append(channel(favoriteIds.at(i)));
        }
        QSqlDatabase::database().commit();
    } else {
        execute(*m_channelsQuery);
        while (m_channelsQuery->next()) {
            ChannelData data;
            data.m_id = ChannelId(m_channelsQuery->value(QStringLiteral("id")).toString());
            data.m_name = m_channelsQuery->value(QStringLiteral("name")).toString();
            data.m_url = m_channelsQuery->value(QStringLiteral("url")).toString();
            data.m_image = m_channelsQuery->value(QStringLiteral("image")).toString();
            channels.append(data);
        }
    }
    return channels;
}

ChannelData Database::channel(const ChannelId &channelId)
{
    ChannelData data;
    data.m_id = channelId;

    m_channelQuery->bindValue(QStringLiteral(":channelId"), data.m_id.value());
    execute(*m_channelQuery);
    if (!m_channelQuery->next()) {
        qWarning() << "Failed to query channel" << channelId.value();
    } else {
        data.m_id = ChannelId(m_channelQuery->value(QStringLiteral("id")).toString());
        data.m_name = m_channelQuery->value(QStringLiteral("name")).toString();
        data.m_url = m_channelQuery->value(QStringLiteral("url")).toString();
        data.m_image = m_channelQuery->value(QStringLiteral("image")).toString();
    }
    return data;
}

void Database::addFavorite(const ChannelId &channelId)
{
    m_addFavoriteQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    execute(*m_addFavoriteQuery);

    Q_EMIT channelDetailsUpdated(channelId, true);
}

void Database::removeFavorite(const ChannelId &channelId)
{
    m_removeFavoriteQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    execute(*m_removeFavoriteQuery);

    Q_EMIT channelDetailsUpdated(channelId, false);
}

void Database::setFavorites(const QVector<ChannelId> &channelIds)
{
    QSqlDatabase::database().transaction();
    clearFavorites();
    for (const auto &channelId : channelIds) {
        addFavorite(channelId);
    }
    QSqlDatabase::database().commit();
}

void Database::clearFavorites()
{
    const QVector<ChannelId> favoriteChannelIds = favorites();

    execute(*m_clearFavoritesQuery);

    for (const auto &channelId : favoriteChannelIds) {
        Q_EMIT channelDetailsUpdated(channelId, false);
    }
}

size_t Database::favoriteCount()
{
    execute(*m_favoriteCountQuery);
    if (!m_favoriteCountQuery->next()) {
        qWarning() << "Failed to query favorite count";
        return 0;
    }
    return m_favoriteCountQuery->value(0).toInt();
}

QVector<ChannelId> Database::favorites()
{
    QVector<ChannelId> favorites;

    execute(*m_favoritesQuery);
    while (m_favoritesQuery->next()) {
        const ChannelId channelId = ChannelId(m_favoritesQuery->value(QStringLiteral("channel")).toString());
        favorites.append(channelId);
    }
    return favorites;
}

bool Database::isFavorite(const ChannelId &channelId)
{
    m_isFavoriteQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    execute(*m_isFavoriteQuery);
    m_isFavoriteQuery->next();
    return m_isFavoriteQuery->value(0).toInt() > 0;
}

void Database::addProgram(const ProgramData &data)
{
    m_addProgramQuery->bindValue(QStringLiteral(":id"), data.m_id.value());
    m_addProgramQuery->bindValue(QStringLiteral(":url"), data.m_url);
    m_addProgramQuery->bindValue(QStringLiteral(":channel"), data.m_channelId.value());
    m_addProgramQuery->bindValue(QStringLiteral(":start"), data.m_startTime.toSecsSinceEpoch());
    m_addProgramQuery->bindValue(QStringLiteral(":stop"), data.m_stopTime.toSecsSinceEpoch());
    m_addProgramQuery->bindValue(QStringLiteral(":title"), data.m_title);
    m_addProgramQuery->bindValue(QStringLiteral(":subtitle"), data.m_subtitle);
    m_addProgramQuery->bindValue(QStringLiteral(":description"), data.m_description);
    m_addProgramQuery->bindValue(QStringLiteral(":category"), data.m_category);

    execute(*m_addProgramQuery);
}

void Database::updateProgramDescription(const ProgramId &id, const QString &description)
{
    m_updateProgramDescriptionQuery->bindValue(QStringLiteral(":id"), id.value());
    m_updateProgramDescriptionQuery->bindValue(QStringLiteral(":description"), description);

    execute(*m_updateProgramDescriptionQuery);
}

void Database::addPrograms(const QVector<ProgramData> &programs)
{
    QSqlDatabase::database().transaction();

    for (int i = 0; i < programs.length(); i++) {
        const ProgramData &data = programs.at(i);
        addProgram(data);
    }

    QSqlDatabase::database().commit();
}

bool Database::programExists(const ChannelId &channelId, qint64 lastTime)
{
    m_programExistsQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    m_programExistsQuery->bindValue(QStringLiteral(":lastTime"), lastTime);
    execute(*m_programExistsQuery);
    m_programExistsQuery->next();

    return m_programExistsQuery->value(0).toInt() > 0;
}

size_t Database::programCount(const ChannelId &channelId)
{
    m_programCountQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    execute(*m_programCountQuery);
    if (!m_programCountQuery->next()) {
        qWarning() << "Failed to query program count";
        return 0;
    }
    return m_programCountQuery->value(0).toInt();
}

QMap<ChannelId, QVector<ProgramData>> Database::programs()
{
    QMap<ChannelId, QVector<ProgramData>> programs;

    execute(*m_programsQuery);

    while (m_programsQuery->next()) {
        const ChannelId channelId = ChannelId(m_programsQuery->value(QStringLiteral("channel")).toString());
        if (!programs.contains(channelId)) {
            programs.insert(channelId, QVector<ProgramData>());
        }

        ProgramData data;
        data.m_id = ProgramId(m_programsQuery->value(QStringLiteral("id")).toString());
        data.m_url = m_programsQuery->value(QStringLiteral("url")).toString();
        data.m_channelId = channelId;
        data.m_startTime.setSecsSinceEpoch(m_programsQuery->value(QStringLiteral("start")).toInt());
        data.m_stopTime.setSecsSinceEpoch(m_programsQuery->value(QStringLiteral("stop")).toInt());
        data.m_title = m_programsQuery->value(QStringLiteral("title")).toString();
        data.m_subtitle = m_programsQuery->value(QStringLiteral("subtitle")).toString();
        data.m_description = m_programsQuery->value(QStringLiteral("description")).toString();
        data.m_category = m_programsQuery->value(QStringLiteral("category")).toString();

        programs[channelId].push_back(data);
    }

    return programs;
}

QVector<ProgramData> Database::programs(const ChannelId &channelId)
{
    QVector<ProgramData> programs;

    m_programsPerChannelQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    execute(*m_programsPerChannelQuery);

    while (m_programsPerChannelQuery->next()) {
        ProgramData data;
        data.m_id = ProgramId(m_programsPerChannelQuery->value(QStringLiteral("id")).toString());
        data.m_url = m_programsPerChannelQuery->value(QStringLiteral("url")).toString();
        data.m_channelId = ChannelId(m_programsPerChannelQuery->value(QStringLiteral("channel")).toString());
        data.m_startTime.setSecsSinceEpoch(m_programsPerChannelQuery->value(QStringLiteral("start")).toInt());
        data.m_stopTime.setSecsSinceEpoch(m_programsPerChannelQuery->value(QStringLiteral("stop")).toInt());
        data.m_title = m_programsPerChannelQuery->value(QStringLiteral("title")).toString();
        data.m_subtitle = m_programsPerChannelQuery->value(QStringLiteral("subtitle")).toString();
        data.m_description = m_programsPerChannelQuery->value(QStringLiteral("description")).toString();
        data.m_category = m_programsPerChannelQuery->value(QStringLiteral("category")).toString();

        programs.push_back(data);
    }
    return programs;
}
