/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <KLocalizedString>
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStandardPaths>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "TellyScoutSettings.h"
#include "database.h"
#include "fetcher.h"

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
}

bool Database::createTables()
{
    qDebug() << "Create DB tables";
    TRUE_OR_RETURN(execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Countries (id TEXT UNIQUE, name TEXT, url TEXT);")));
    TRUE_OR_RETURN(
        execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Channels (id TEXT UNIQUE, name TEXT, url TEXT, image TEXT, notify BOOL, favorite BOOL);")));
    TRUE_OR_RETURN(
        execute(QStringLiteral("CREATE TABLE IF NOT EXISTS Programs (id TEXT UNIQUE, channel TEXT, start INTEGER, stop INTEGER, title TEXT, subtitle TEXT, "
                               "description TEXT, category TEXT);")));
    TRUE_OR_RETURN(execute(
        QStringLiteral("CREATE TABLE IF NOT EXISTS Enclosures (channel TEXT, id TEXT, duration INTEGER, size INTEGER, title TEXT, type STRING, url STRING);")));
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
    TellyScoutSettings settings;
    int count = settings.deleteAfterCount();
    int type = settings.deleteAfterType();

    if (type == 0) { // Never delete Programs
        return;
    }

    if (type == 1) { // Delete after <count> posts per channel
        // TODO
    } else {
        QDateTime dateTime = QDateTime::currentDateTime();
        if (type == 2) {
            dateTime = dateTime.addDays(-count);
        } else if (type == 3) {
            dateTime = dateTime.addDays(-7 * count);
        } else if (type == 4) {
            dateTime = dateTime.addMonths(-count);
        }
        qint64 sinceEpoch = dateTime.toSecsSinceEpoch();

        QSqlQuery query;
        query.prepare(QStringLiteral("DELETE FROM Programs WHERE updated < :sinceEpoch;"));
        query.bindValue(QStringLiteral(":sinceEpoch"), sinceEpoch);
        execute(query);
    }
}

bool Database::channelExists(const QString &url)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (url) FROM Channels WHERE url=:url;"));
    query.bindValue(QStringLiteral(":url"), url);
    Database::instance().execute(query);
    query.next();
    return query.value(0).toInt() != 0;
}

void Database::addChannel(const QString &url, bool favorite)
{
    qDebug() << "Adding channel";
    if (channelExists(url)) {
        qDebug() << "Channel already exists";
        return;
    }
    qDebug() << "Channel does not yet exist";

    QUrl urlFromInput = QUrl::fromUserInput(url);
    QSqlQuery query;
    query.prepare(
        QStringLiteral("INSERT INTO Channels VALUES (:name, :url, :image, :link, :description, :deleteAfterCount, :deleteAfterType, :subscribed, :lastUpdated, "
                       ":notify, :favorite, :displayName);"));
    query.bindValue(QStringLiteral(":name"), urlFromInput.toString());
    query.bindValue(QStringLiteral(":url"), urlFromInput.toString());
    query.bindValue(QStringLiteral(":image"), QLatin1String(""));
    query.bindValue(QStringLiteral(":link"), QLatin1String(""));
    query.bindValue(QStringLiteral(":description"), QLatin1String(""));
    query.bindValue(QStringLiteral(":deleteAfterCount"), 0);
    query.bindValue(QStringLiteral(":deleteAfterType"), 0);
    query.bindValue(QStringLiteral(":subscribed"), QDateTime::currentDateTime().toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":lastUpdated"), 0);
    query.bindValue(QStringLiteral(":notify"), false);
    query.bindValue(QStringLiteral(":favorite"), favorite);
    query.bindValue(QStringLiteral(":displayName"), QLatin1String(""));
    execute(query);

    Q_EMIT channelAdded(urlFromInput.toString());

    Fetcher::instance().fetchChannel(urlFromInput.toString(), urlFromInput.toString()); // TODO: url -> ID
}

void Database::importChannels(const QString &path)
{
    QUrl url(path);
    QFile file(url.isLocalFile() ? url.toLocalFile() : url.toString());
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader xmlReader(&file);
    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.tokenType() == 4 && xmlReader.attributes().hasAttribute(QStringLiteral("xmlUrl"))) {
            addChannel(xmlReader.attributes().value(QStringLiteral("xmlUrl")).toString());
        }
    }
    Fetcher::instance().fetchAll();
}

void Database::exportChannels(const QString &path)
{
    QUrl url(path);
    QFile file(url.isLocalFile() ? url.toLocalFile() : url.toString());
    file.open(QIODevice::WriteOnly);

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument(QStringLiteral("1.0"));
    xmlWriter.writeStartElement(QStringLiteral("opml"));
    xmlWriter.writeEmptyElement(QStringLiteral("head"));
    xmlWriter.writeStartElement(QStringLiteral("body"));
    xmlWriter.writeAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT url, name FROM Channels;"));
    execute(query);
    while (query.next()) {
        xmlWriter.writeEmptyElement(QStringLiteral("outline"));
        xmlWriter.writeAttribute(QStringLiteral("xmlUrl"), query.value(0).toString());
        xmlWriter.writeAttribute(QStringLiteral("title"), query.value(1).toString());
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
}

void Database::editChannel(const QString &url, const QString &displayName, bool favorite)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE Channels SET displayName = :displayName, favorite = :favorite WHERE url = :url;"));
    query.bindValue(QStringLiteral(":displayName"), displayName);
    query.bindValue(QStringLiteral(":favorite"), favorite);
    query.bindValue(QStringLiteral(":url"), url);
    execute(query);

    Q_EMIT channelDetailsUpdated(url, displayName, favorite);
}
