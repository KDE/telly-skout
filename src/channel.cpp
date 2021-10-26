/*
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "channel.h"

#include "database.h"
#include "fetcher.h"
#include "program.h"
#include "programsmodel.h"

#include <QDebug>
#include <QSqlQuery>

Channel::Channel(int index, bool onlyFavorites)
    : QObject(nullptr)
{
    QSqlQuery query;
    if (onlyFavorites) {
        QSqlQuery favoriteQuery;
        favoriteQuery.prepare(QStringLiteral("SELECT * FROM Favorites ORDER BY id LIMIT 1 OFFSET :index;"));
        favoriteQuery.bindValue(QStringLiteral(":index"), index);
        Database::instance().execute(favoriteQuery);
        if (!favoriteQuery.next()) {
            qWarning() << "Failed to load favorite channel" << index;
        }
        const QString channelId = favoriteQuery.value(QStringLiteral("channel")).toString();
        m_favorite = true;

        query.prepare(QStringLiteral("SELECT * FROM Channels WHERE id=:channelId"));
        query.bindValue(QStringLiteral(":channelId"), channelId);
    } else {
        query.prepare(QStringLiteral("SELECT * FROM Channels ORDER BY name COLLATE NOCASE LIMIT 1 OFFSET :index;"));
        query.bindValue(QStringLiteral(":index"), index);
    }
    Database::instance().execute(query);
    if (!query.next()) {
        qWarning() << "Failed to load channel" << index;
    }

    m_id = query.value(QStringLiteral("id")).toString();
    m_url = query.value(QStringLiteral("url")).toString();
    m_name = query.value(QStringLiteral("name")).toString();
    m_image = query.value(QStringLiteral("image")).toString();
    m_notify = query.value(QStringLiteral("notify")).toBool();

    if (!onlyFavorites) {
        QSqlQuery favoriteQuery;
        favoriteQuery.prepare(QStringLiteral("SELECT * FROM Favorites WHERE channel=:channel"));
        favoriteQuery.bindValue(QStringLiteral(":channel"), m_id);
        Database::instance().execute(favoriteQuery);
        m_favorite = favoriteQuery.next();
    }

    QSqlQuery countryQuery;
    countryQuery.prepare(QStringLiteral("SELECT * FROM CountryChannels WHERE channel=:channel"));
    countryQuery.bindValue(QStringLiteral(":channel"), m_id);
    Database::instance().execute(countryQuery);
    while (countryQuery.next()) {
        m_countries.push_back(countryQuery.value(QStringLiteral("country")).toString());
    }

    m_errorId = 0;
    m_errorString = QLatin1String("");

    connect(&Fetcher::instance(), &Fetcher::startedFetchingChannel, this, [this](const QString &id) {
        if (id == m_id) {
            setRefreshing(true);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::channelUpdated, this, [this](const QString &id) {
        if (id == m_id) {
            setRefreshing(false);
            Q_EMIT programChanged();
            setErrorId(0);
            setErrorString(QLatin1String(""));
        }
    });
    connect(&Fetcher::instance(), &Fetcher::error, this, [this](const QString &id, int errorId, const QString &errorString) {
        if (id == m_id) {
            setErrorId(errorId);
            setErrorString(errorString);
            setRefreshing(false);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::imageDownloadFinished, this, [this](const QString &url) {
        if (url == m_image) {
            Q_EMIT imageChanged(url);
        }
    });

    // programs
    m_programsModel = new ProgramsModel(this);
}

Channel::~Channel()
{
}

QString Channel::id() const
{
    return m_id;
}

QString Channel::url() const
{
    return m_url;
}

QString Channel::name() const
{
    return m_name;
}

QString Channel::image() const
{
    return m_image;
}

bool Channel::favorite() const
{
    return m_favorite;
}

QVector<QString> Channel::countries() const
{
    return m_countries;
}

bool Channel::notify() const
{
    return m_notify;
}

bool Channel::refreshing() const
{
    return m_refreshing;
}

int Channel::errorId() const
{
    return m_errorId;
}

QString Channel::errorString() const
{
    return m_errorString;
}

void Channel::setName(const QString &name)
{
    m_name = name;
    Q_EMIT nameChanged(m_name);
}

void Channel::setImage(const QString &image)
{
    m_image = image;
    Q_EMIT imageChanged(m_image);
}

void Channel::setFavorite(bool favorite)
{
    if (m_favorite != favorite) {
        m_favorite = favorite;

        Q_EMIT favoriteChanged(favorite);
    }
}

void Channel::setCountries(const QVector<QString> &countries)
{
    m_countries = countries;
    Q_EMIT countriesChanged(m_countries);
}

void Channel::setNotify(bool notify)
{
    m_notify = notify;
    Q_EMIT notifyChanged(m_notify);
}

void Channel::setRefreshing(bool refreshing)
{
    m_refreshing = refreshing;
    Q_EMIT refreshingChanged(m_refreshing);
}

void Channel::setErrorId(int errorId)
{
    m_errorId = errorId;
    Q_EMIT errorIdChanged(m_errorId);
}

void Channel::setErrorString(const QString &errorString)
{
    m_errorString = errorString;
    Q_EMIT errorStringChanged(m_errorString);
}

void Channel::refresh()
{
    Fetcher::instance().fetchChannel(m_url, m_url, ""); // TODO: url -> ID
}

void Channel::setAsFavorite(bool favorite)
{
    if (favorite) {
        unsigned int id = 1;
        QSqlQuery idQuery;
        idQuery.prepare(QStringLiteral("SELECT id FROM Favorites ORDER BY id DESC LIMIT 1;"));
        Database::instance().execute(idQuery);
        if (idQuery.next()) {
            id = idQuery.value(QStringLiteral("id")).toUInt() + 1;
        }
        QSqlQuery query;
        query.prepare(QStringLiteral("INSERT INTO Favorites VALUES (:id, :channel);"));
        query.bindValue(QStringLiteral(":id"), id);
        query.bindValue(QStringLiteral(":channel"), m_id);
        Database::instance().execute(query);
    } else {
        QSqlQuery query;
        query.prepare(QStringLiteral("DELETE FROM Favorites WHERE channel=:channel;"));
        query.bindValue(QStringLiteral(":channel"), m_id);
        Database::instance().execute(query);
    }
}

void Channel::remove()
{
    // Delete Countries
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM Countries WHERE channel=:channel;"));
    query.bindValue(QStringLiteral(":channel"), m_url);
    Database::instance().execute(query);

    // Delete Programs
    query.prepare(QStringLiteral("DELETE FROM Programs WHERE channel=:channel;"));
    query.bindValue(QStringLiteral(":channel"), m_url);
    Database::instance().execute(query);

    // Delete Favorite
    query.prepare(QStringLiteral("DELETE FROM Favorites WHERE channel=:channel;"));
    query.bindValue(QStringLiteral(":channel"), m_id);
    Database::instance().execute(query);

    // Delete Channel
    query.prepare(QStringLiteral("DELETE FROM Channels WHERE url=:url;"));
    query.bindValue(QStringLiteral(":url"), m_url);
    Database::instance().execute(query);
}
