/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

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
    QString databasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
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
    m_addCountryChannelQuery = new QSqlQuery(db);
    m_addCountryChannelQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO CountryChannels VALUES (:id, :country, :channel);"));
    m_addChannelQuery = new QSqlQuery(db);
    m_addChannelQuery->prepare(QStringLiteral("INSERT OR IGNORE INTO Channels VALUES (:id, :name, :url, :image, :notify);"));
    m_addProgramQuery = new QSqlQuery(db);
    m_addProgramQuery->prepare(
        QStringLiteral("INSERT OR IGNORE INTO Programs VALUES (:id, :url, :channel, :start, :stop, :title, :subtitle, :description, :category);"));
    m_updateProgramDescriptionQuery = new QSqlQuery(db);
    m_updateProgramDescriptionQuery->prepare(QStringLiteral("UPDATE Programs SET description=:description WHERE id=:id;"));
    m_programExistsQuery = new QSqlQuery(db);
    m_programExistsQuery->prepare(QStringLiteral("SELECT COUNT (id) FROM Programs WHERE channel=:channel AND stop>=:lastTime;"));
    m_programsQuery = new QSqlQuery(db);
    m_programsQuery->prepare(QStringLiteral("SELECT * FROM Programs WHERE channel=:channel ORDER BY start;"));
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
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Channels (id TEXT UNIQUE, name TEXT, url TEXT, image TEXT, notify BOOL);")));
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

void Database::addCountry(const QString &id, const QString &name, const QString &url)
{
    qDebug() << "Add country" << name;

    QUrl urlFromInput = QUrl::fromUserInput(url);
    m_addCountryQuery->bindValue(QStringLiteral(":id"), id);
    m_addCountryQuery->bindValue(QStringLiteral(":name"), name);
    m_addCountryQuery->bindValue(QStringLiteral(":url"), urlFromInput.toString());
    execute(*m_addCountryQuery);

    Q_EMIT countryAdded(urlFromInput.toString());
}

void Database::addChannel(const QString &id, const QString &name, const QString &url, const QString &country, const QString &image)
{
    qDebug() << "Add channel" << name;

    // store channel per country (ignore if it exists already)
    {
        m_addCountryChannelQuery->bindValue(QStringLiteral(":id"), country + "_" + id);
        m_addCountryChannelQuery->bindValue(QStringLiteral(":country"), country);
        m_addCountryChannelQuery->bindValue(QStringLiteral(":channel"), id);
        execute(*m_addCountryChannelQuery);
    }

    // store channel (ignore if it exists already)
    {
        QUrl urlFromInput = QUrl::fromUserInput(url);
        m_addChannelQuery->bindValue(QStringLiteral(":id"), id);
        m_addChannelQuery->bindValue(QStringLiteral(":name"), name);
        m_addChannelQuery->bindValue(QStringLiteral(":url"), urlFromInput.toString());
        m_addChannelQuery->bindValue(QStringLiteral(":country"), country);
        m_addChannelQuery->bindValue(QStringLiteral(":image"), image);
        m_addChannelQuery->bindValue(QStringLiteral(":notify"), false);
        execute(*m_addChannelQuery);
        Q_EMIT channelAdded(urlFromInput.toString()); // TODO use id
    }
}

void Database::addProgram(const QString &id,
                          const QString &url,
                          const QString &channelId,
                          const QDateTime &startTime,
                          const QDateTime &stopTime,
                          const QString &title,
                          const QString &subtitle,
                          const QString &description,
                          const QString &category)
{
    m_addProgramQuery->bindValue(QStringLiteral(":id"), id);
    m_addProgramQuery->bindValue(QStringLiteral(":url"), url);
    m_addProgramQuery->bindValue(QStringLiteral(":channel"), channelId);
    m_addProgramQuery->bindValue(QStringLiteral(":start"), startTime.toSecsSinceEpoch());
    m_addProgramQuery->bindValue(QStringLiteral(":stop"), stopTime.toSecsSinceEpoch());
    m_addProgramQuery->bindValue(QStringLiteral(":title"), title);
    m_addProgramQuery->bindValue(QStringLiteral(":subtitle"), subtitle); // TODO
    m_addProgramQuery->bindValue(QStringLiteral(":description"), description); // set in fetchDescription()
    m_addProgramQuery->bindValue(QStringLiteral(":category"), category);

    execute(*m_addProgramQuery);
}

void Database::updateProgramDescription(const QString &id, const QString &description)
{
    m_updateProgramDescriptionQuery->bindValue(QStringLiteral(":id"), id);
    m_updateProgramDescriptionQuery->bindValue(QStringLiteral(":description"), description);

    execute(*m_updateProgramDescriptionQuery);
}

void Database::addPrograms(const QVector<ProgramData> &programs)
{
    QSqlDatabase::database().transaction();

    for (int i = 0; i < programs.length(); i++) {
        const ProgramData &programData = programs.at(i);
        addProgram(programData.m_id,
                   programData.m_url,
                   programData.m_channelId,
                   programData.m_startTime,
                   programData.m_stopTime,
                   programData.m_title,
                   programData.m_subtitle,
                   programData.m_description,
                   programData.m_category);
    }

    QSqlDatabase::database().commit();
}

QVector<QString> Database::favoriteChannels()
{
    QVector<QString> favorites;

    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT channel FROM Favorites;"));
    execute(query);
    while (query.next()) {
        const QString &channelId = query.value(QStringLiteral("channel")).toString();
        favorites.append(channelId);
    }
    return favorites;
}

bool Database::programExists(const QString &channelId, qint64 lastTime)
{
    m_programExistsQuery->bindValue(QStringLiteral(":channel"), channelId);
    m_programExistsQuery->bindValue(QStringLiteral(":lastTime"), lastTime);
    execute(*m_programExistsQuery);
    m_programExistsQuery->next();

    return m_programExistsQuery->value(0).toInt() > 0;
}

QVector<ProgramData> Database::programs(const QString &channelId)
{
    QVector<ProgramData> programs;

    m_programsQuery->bindValue(QStringLiteral(":channel"), channelId);
    execute(*m_programsQuery);

    while (m_programsQuery->next()) {
        ProgramData data;
        data.m_id = m_programsQuery->value(QStringLiteral("id")).toString();
        data.m_url = m_programsQuery->value(QStringLiteral("url")).toString();
        data.m_channelId = m_programsQuery->value(QStringLiteral("channel")).toString();
        data.m_startTime.setSecsSinceEpoch(m_programsQuery->value(QStringLiteral("start")).toInt());
        data.m_stopTime.setSecsSinceEpoch(m_programsQuery->value(QStringLiteral("stop")).toInt());
        data.m_title = m_programsQuery->value(QStringLiteral("title")).toString();
        data.m_subtitle = m_programsQuery->value(QStringLiteral("subtitle")).toString();
        data.m_description = m_programsQuery->value(QStringLiteral("description")).toString();
        data.m_category = m_programsQuery->value(QStringLiteral("category")).toString();

        programs.push_back(data);
    }
    return programs;
}
