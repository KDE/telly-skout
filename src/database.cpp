// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: GPL-3.0-only

#include "database.h"

#include <QDateTime>
#include <QDebug>
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
    if (!db.open()) {
        qCritical() << "Failed to open database";
    }

    // drop DB if it doesn't use the correct fetcher
    if (m_settings.fetcher() != fetcher()) {
        if (!dropTables()) {
            qCritical() << "Failed to drop database";
        }
    }

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
    m_addGroupQuery.reset(new QSqlQuery(db));
    bool success = m_addGroupQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO \"Groups\" VALUES (:id, :name, :url);"));
    m_groupCountQuery.reset(new QSqlQuery(db));
    success &= m_groupCountQuery->prepare(QStringLiteral("SELECT COUNT() FROM \"Groups\";"));
    m_groupExistsQuery.reset(new QSqlQuery(db));
    success &= m_groupExistsQuery->prepare(QStringLiteral("SELECT COUNT () FROM \"Groups\" WHERE id=:id;"));
    m_groupsQuery.reset(new QSqlQuery(db));
    success &= m_groupsQuery->prepare(QStringLiteral("SELECT * FROM \"Groups\" ORDER BY name COLLATE NOCASE;"));
    m_groupsPerChannelQuery.reset(new QSqlQuery(db));
    success &= m_groupsPerChannelQuery->prepare(
        QStringLiteral("SELECT * FROM \"Groups\" WHERE id=(SELECT \"group\" from GroupChannels WHERE channel=:channel) ORDER BY name COLLATE NOCASE;"));

    m_addGroupChannelQuery.reset(new QSqlQuery(db));
    success &= m_addGroupChannelQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO GroupChannels VALUES (:id, :group, :channel);"));

    m_addFavoriteQuery.reset(new QSqlQuery(db));
    success &= m_addFavoriteQuery->prepare(QStringLiteral("INSERT INTO Favorites VALUES ((SELECT COUNT() FROM Favorites) + 1, :channel);"));
    m_addChannelQuery.reset(new QSqlQuery(db));
    success &= m_addChannelQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO Channels VALUES (:id, :name, :url, :image);"));
    m_channelCountQuery.reset(new QSqlQuery(db));
    success &= m_channelCountQuery->prepare(QStringLiteral("SELECT COUNT() FROM Channels;"));
    m_channelExistsQuery.reset(new QSqlQuery(db));
    success &= m_channelExistsQuery->prepare(QStringLiteral("SELECT COUNT () FROM Channels WHERE id=:id;"));
    m_channelsQuery.reset(new QSqlQuery(db));
    success &= m_channelsQuery->prepare(QStringLiteral("SELECT * FROM Channels ORDER BY name COLLATE NOCASE;"));
    m_channelQuery.reset(new QSqlQuery(db));
    success &= m_channelQuery->prepare(QStringLiteral("SELECT * FROM Channels WHERE id=:channelId;"));

    m_clearFavoritesQuery.reset(new QSqlQuery(db));
    success &= m_clearFavoritesQuery->prepare(QStringLiteral("DELETE FROM Favorites;"));
    m_favoriteCountQuery.reset(new QSqlQuery(db));
    success &= m_favoriteCountQuery->prepare(QStringLiteral("SELECT COUNT() FROM Favorites;"));
    m_favoritesQuery.reset(new QSqlQuery(db));
    success &= m_favoritesQuery->prepare(QStringLiteral("SELECT channel FROM Favorites ORDER BY id;"));
    m_isFavoriteQuery.reset(new QSqlQuery(db));
    success &= m_isFavoriteQuery->prepare(QStringLiteral("SELECT COUNT() FROM Favorites WHERE channel=:channel"));

    m_addProgramQuery.reset(new QSqlQuery(db));
    success &= m_addProgramQuery->prepare(
        QStringLiteral("INSERT OR IGNORE INTO Programs VALUES (:id, :url, :channel, :start, :stop, :title, :subtitle, :description, :descriptionFetched);"));
    m_updateProgramDescriptionQuery.reset(new QSqlQuery(db));
    success &= m_updateProgramDescriptionQuery->prepare(QStringLiteral("UPDATE Programs SET description=:description, descriptionFetched=TRUE WHERE id=:id;"));
    m_programExistsQuery.reset(new QSqlQuery(db));
    success &= m_programExistsQuery->prepare(QStringLiteral("SELECT COUNT () FROM Programs WHERE channel=:channel AND stop>=:lastTime;"));
    m_programCountQuery.reset(new QSqlQuery(db));
    success &= m_programCountQuery->prepare(QStringLiteral("SELECT COUNT() FROM Programs WHERE channel=:channel;"));
    m_programsQuery.reset(new QSqlQuery(db));
    success &= m_programsQuery->prepare(QStringLiteral("SELECT * FROM Programs ORDER BY channel, start;"));
    m_programsPerChannelQuery.reset(new QSqlQuery(db));
    success &= m_programsPerChannelQuery->prepare(QStringLiteral("SELECT * FROM Programs WHERE channel=:channel ORDER BY start;"));

    m_addProgramCategoryQuery.reset(new QSqlQuery(db));
    success &= m_addProgramCategoryQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO ProgramCategories VALUES (:program, :category);"));
    m_programCategoriesQuery.reset(new QSqlQuery(db));
    success &= m_programCategoriesQuery->prepare(QStringLiteral("SELECT category FROM ProgramCategories WHERE program=:program;"));

    if (!success) {
        qCritical() << "Failed to prepare database queries";
    }

    connect(&m_settings, &TellySkoutSettings::fetcherChanged, this, [this]() {
        dropTables();
        createTables();
    });
}

bool Database::createTables()
{
    qDebug() << "Create DB tables";

    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Fetcher (id INTEGER UNIQUE);")));
    TRUE_OR_RETURN(execute(QStringLiteral("INSERT OR IGNORE INTO Fetcher VALUES (") + QString::number(m_settings.fetcher()) + ");"));

    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS \"Groups\" (id TEXT UNIQUE, name TEXT, url TEXT);")));
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Channels (id TEXT UNIQUE, name TEXT, url TEXT, image TEXT);")));
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS GroupChannels (id TEXT UNIQUE, \"Group\" TEXT, channel TEXT);")));
    TRUE_OR_RETURN(execute(
        QStringLiteral("CREATE TABLE IF NOT EXISTS Programs (id TEXT UNIQUE, url TEXT, channel TEXT, start INTEGER, stop INTEGER, title TEXT, subtitle TEXT, "
                       "description TEXT, descriptionFetched INTEGER);")));
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS ProgramCategories (program TEXT, category TEXT);")));
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Favorites (id INTEGER UNIQUE, channel TEXT UNIQUE);")));

    TRUE_OR_RETURN(execute(QStringLiteral("PRAGMA user_version = 1;")));
    return true;
}

bool Database::dropTables()
{
    qDebug() << "Drop DB tables";
    TRUE_OR_RETURN(execute(QStringLiteral("DROP TABLE IF EXISTS Fetcher;")));
    TRUE_OR_RETURN(execute(QStringLiteral("DROP TABLE IF EXISTS \"Groups\";")));
    TRUE_OR_RETURN(execute(QStringLiteral("DROP TABLE IF EXISTS Channels;")));
    TRUE_OR_RETURN(execute(QStringLiteral("DROP TABLE IF EXISTS GroupChannels;")));
    TRUE_OR_RETURN(execute(QStringLiteral("DROP TABLE IF EXISTS Programs;")));
    TRUE_OR_RETURN(execute(QStringLiteral("DROP TABLE IF EXISTS ProgramCategories;")));
    TRUE_OR_RETURN(execute(QStringLiteral("DROP TABLE IF EXISTS Favorites;")));

    return true;
}

bool Database::execute(const QString &query) const
{
    QSqlQuery q;
    if (q.prepare(query)) {
        return execute(q);
    } else {
        qCritical() << "Failed to prepare query '" << query << "'";
        return false;
    }
}

bool Database::execute(QSqlQuery &query) const
{
    if (!query.exec()) {
        qWarning() << "Failed to execute SQL Query";
        qWarning() << query.lastQuery();
        qWarning() << query.lastError();
        return false;
    }
    return true;
}

int Database::version() const
{
    const int error = -1;

    QSqlQuery query;
    if (!query.prepare(QStringLiteral("PRAGMA user_version;"))) {
        qCritical() << "Failed to prepare query for user_version";
        return error;
    }
    if (!execute(query)) {
        qCritical() << "Failed to execute query for user_version";
        return error;
    }
    if (query.next()) {
        bool ok;
        int value = query.value(0).toInt(&ok);
        qDebug() << "Database version" << value;
        if (ok) {
            return value;
        }
    } else {
        qCritical() << "Failed to check database version";
    }
    return error;
}

int Database::fetcher() const
{
    const int error = -1;

    QSqlQuery query;
    if (!query.prepare(QStringLiteral("SELECT * FROM Fetcher;"))) {
        qCritical() << "Failed to prepare query for fetcher";
        return error;
    }
    if (!execute(query)) {
        qCritical() << "Failed to execute query for fetcher";
        return error;
    }
    if (query.next()) {
        bool ok;
        int value = query.value(0).toInt(&ok);
        qDebug() << "Database for fetcher" << value;
        if (ok) {
            return value;
        }
    } else {
        qCritical() << "Failed to check fetcher";
    }
    return error;
}

void Database::cleanup()
{
    // delete programs in the past to avoid that the database grows over time
    const unsigned int daysPast = m_settings.deleteProgramAfter();
    QDateTime dateTimePast = QDateTime::currentDateTime();
    dateTimePast = dateTimePast.addDays(-static_cast<qint64>(daysPast));

    // delete programs in the far future (probably they have been added by mistake)
    QDateTime dateTimeFuture = QDateTime::currentDateTime();
    dateTimeFuture = dateTimeFuture.addDays(30);

    QSqlQuery query;
    if (!query.prepare(QStringLiteral("DELETE FROM Programs WHERE stop < :sinceEpochPast OR stop > :sinceEpochFuture;"))) {
        qCritical() << "Failed to prepare cleanup query";
        return;
    }
    query.bindValue(QStringLiteral(":sinceEpochPast"), dateTimePast.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":sinceEpochFuture"), dateTimeFuture.toSecsSinceEpoch());
    execute(query);
}

void Database::addGroup(const GroupId &id, const QString &name, const QString &url)
{
    if (!groupExists(id)) {
        qDebug() << "Add group" << name;
        m_addGroupQuery->bindValue(QStringLiteral(":id"), id.value());
        m_addGroupQuery->bindValue(QStringLiteral(":name"), name);
        m_addGroupQuery->bindValue(QStringLiteral(":url"), url);
        execute(*m_addGroupQuery);

        Q_EMIT groupAdded(id);
    }
}

size_t Database::groupCount() const
{
    execute(*m_groupCountQuery);
    if (!m_groupCountQuery->next()) {
        qWarning() << "Failed to query group count";
        return 0;
    }
    return m_groupCountQuery->value(0).toUInt();
}

bool Database::groupExists(const GroupId &id) const
{
    m_groupExistsQuery->bindValue(QStringLiteral(":id"), id.value());
    execute(*m_groupExistsQuery);
    m_groupExistsQuery->next();

    return m_groupExistsQuery->value(0).toInt() > 0;
}

QVector<GroupData> Database::groups() const
{
    QVector<GroupData> groups;

    execute(*m_groupsQuery);
    while (m_groupsQuery->next()) {
        GroupData data;
        data.m_id = GroupId(m_groupsQuery->value(QStringLiteral("id")).toString());
        data.m_name = m_groupsQuery->value(QStringLiteral("name")).toString();
        data.m_url = m_groupsQuery->value(QStringLiteral("url")).toString();
        groups.append(data);
    }
    return groups;
}

QVector<GroupData> Database::groups(const ChannelId &channelId) const
{
    QVector<GroupData> groups;

    m_groupsPerChannelQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    execute(*m_groupsPerChannelQuery);
    while (m_groupsPerChannelQuery->next()) {
        GroupData data;
        data.m_id = GroupId(m_groupsPerChannelQuery->value(QStringLiteral("id")).toString());
        data.m_name = m_groupsPerChannelQuery->value(QStringLiteral("name")).toString();
        data.m_url = m_groupsPerChannelQuery->value(QStringLiteral("url")).toString();
        groups.append(data);
    }
    return groups;
}

void Database::addChannel(const ChannelData &data, const GroupId &group)
{
    if (!channelExists(data.m_id)) {
        qDebug() << "Add channel" << data.m_name;

        // store channel per group
        {
            m_addGroupChannelQuery->bindValue(QStringLiteral(":id"), group.value() + "_" + data.m_id.value());
            m_addGroupChannelQuery->bindValue(QStringLiteral(":group"), group.value());
            m_addGroupChannelQuery->bindValue(QStringLiteral(":channel"), data.m_id.value());
            execute(*m_addGroupChannelQuery);
        }

        // store channel
        {
            QUrl urlFromInput = QUrl::fromUserInput(data.m_url);
            m_addChannelQuery->bindValue(QStringLiteral(":id"), data.m_id.value());
            m_addChannelQuery->bindValue(QStringLiteral(":name"), data.m_name);
            m_addChannelQuery->bindValue(QStringLiteral(":url"), urlFromInput.toString());
            m_addChannelQuery->bindValue(QStringLiteral(":group"), group.value());
            m_addChannelQuery->bindValue(QStringLiteral(":image"), data.m_image);
            execute(*m_addChannelQuery);
            Q_EMIT channelAdded(data.m_id);
        }
    }
}

size_t Database::channelCount() const
{
    execute(*m_channelCountQuery);
    if (!m_channelCountQuery->next()) {
        qWarning() << "Failed to query channel count";
        return 0;
    }
    return m_channelCountQuery->value(0).toUInt();
}

bool Database::channelExists(const ChannelId &id) const
{
    m_channelExistsQuery->bindValue(QStringLiteral(":id"), id.value());
    execute(*m_channelExistsQuery);
    m_channelExistsQuery->next();

    return m_channelExistsQuery->value(0).toInt() > 0;
}

QVector<ChannelData> Database::channels(bool onlyFavorites) const
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

ChannelData Database::channel(const ChannelId &channelId) const
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
    // just removing channelId from the Favorites table does not work
    // it would leave gaps in the IDs (making it impossible to add new favorites)
    QVector<ChannelId> favoriteChannelIds = favorites();
    favoriteChannelIds.removeAll(channelId);

    QSqlDatabase::database().transaction();
    execute(*m_clearFavoritesQuery);
    for (const auto &id : qAsConst(favoriteChannelIds)) {
        m_addFavoriteQuery->bindValue(QStringLiteral(":channel"), id.value());
        execute(*m_addFavoriteQuery);
    }
    QSqlDatabase::database().commit();

    Q_EMIT channelDetailsUpdated(channelId, false);
}

void Database::sortFavorites(const QVector<ChannelId> &newOrder)
{
    QSqlDatabase::database().transaction();
    // do not use clearFavorites() and addFavorite() to avoid unneccesary signals (and therefore updates)
    execute(*m_clearFavoritesQuery);
    for (const auto &channelId : newOrder) {
        m_addFavoriteQuery->bindValue(QStringLiteral(":channel"), channelId.value());
        execute(*m_addFavoriteQuery);
    }
    QSqlDatabase::database().commit();

    Q_EMIT favoritesUpdated();
}

void Database::clearFavorites()
{
    const QVector<ChannelId> favoriteChannelIds = favorites();

    execute(*m_clearFavoritesQuery);

    for (const auto &channelId : favoriteChannelIds) {
        Q_EMIT channelDetailsUpdated(channelId, false);
    }
}

size_t Database::favoriteCount() const
{
    execute(*m_favoriteCountQuery);
    if (!m_favoriteCountQuery->next()) {
        qWarning() << "Failed to query favorite count";
        return 0;
    }
    return m_favoriteCountQuery->value(0).toUInt();
}

QVector<ChannelId> Database::favorites() const
{
    QVector<ChannelId> favorites;

    execute(*m_favoritesQuery);
    while (m_favoritesQuery->next()) {
        const ChannelId channelId = ChannelId(m_favoritesQuery->value(QStringLiteral("channel")).toString());
        favorites.append(channelId);
    }
    return favorites;
}

bool Database::isFavorite(const ChannelId &channelId) const
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
    m_addProgramQuery->bindValue(QStringLiteral(":descriptionFetched"), data.m_descriptionFetched);

    execute(*m_addProgramQuery);

    m_addProgramCategoryQuery->bindValue(QStringLiteral(":program"), data.m_id.value());

    const QVector<QString> &categories = data.m_categories;
    for (int i = 0; i < categories.size(); ++i) {
        m_addProgramCategoryQuery->bindValue(QStringLiteral(":category"), categories.at(i));
        execute(*m_addProgramCategoryQuery);
    }
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

bool Database::programExists(const ChannelId &channelId, const QDateTime &lastTime) const
{
    m_programExistsQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    m_programExistsQuery->bindValue(QStringLiteral(":lastTime"), lastTime.toSecsSinceEpoch());
    execute(*m_programExistsQuery);
    m_programExistsQuery->next();

    return m_programExistsQuery->value(0).toInt() > 0;
}

size_t Database::programCount(const ChannelId &channelId) const
{
    m_programCountQuery->bindValue(QStringLiteral(":channel"), channelId.value());
    execute(*m_programCountQuery);
    if (!m_programCountQuery->next()) {
        qWarning() << "Failed to query program count";
        return 0;
    }
    return m_programCountQuery->value(0).toUInt();
}

QMap<ChannelId, QVector<ProgramData>> Database::programs() const
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
        data.m_descriptionFetched = m_programsQuery->value(QStringLiteral("descriptionFetched")).toBool();

        m_programCategoriesQuery->bindValue(QStringLiteral(":program"), data.m_id.value());
        execute(*m_programCategoriesQuery);

        while (m_programCategoriesQuery->next()) {
            data.m_categories.push_back(m_programCategoriesQuery->value(QStringLiteral("category")).toString());
        }

        programs[channelId].push_back(data);
    }

    return programs;
}

QVector<ProgramData> Database::programs(const ChannelId &channelId) const
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
        data.m_descriptionFetched = m_programsPerChannelQuery->value(QStringLiteral("descriptionFetched")).toBool();

        m_programCategoriesQuery->bindValue(QStringLiteral(":program"), data.m_id.value());
        execute(*m_programCategoriesQuery);

        while (m_programCategoriesQuery->next()) {
            data.m_categories.push_back(m_programCategoriesQuery->value(QStringLiteral("category")).toString());
        }

        programs.push_back(data);
    }
    return programs;
}
