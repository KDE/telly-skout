/*
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QVariant>

#include "channel.h"
#include "database.h"
#include "fetcher.h"
#include "program.h"

Channel::Channel(int index, bool onlyFavorite)
    : QObject(nullptr)
{
    QString filterFavorite = "";
    if (onlyFavorite) {
        filterFavorite = "WHERE favorite IS TRUE";
    }
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT * FROM Channels %1 ORDER BY name COLLATE NOCASE LIMIT 1 OFFSET :index;").arg(filterFavorite));
    query.bindValue(QStringLiteral(":index"), index);
    Database::instance().execute(query);
    if (!query.next()) {
        qWarning() << "Failed to load channel" << index;
    }

    m_id = query.value(QStringLiteral("id")).toString();
    m_url = query.value(QStringLiteral("url")).toString();
    m_name = query.value(QStringLiteral("name")).toString();
    m_image = query.value(QStringLiteral("image")).toString();
    m_notify = query.value(QStringLiteral("notify")).toBool();
    m_favorite = query.value(QStringLiteral("favorite")).toBool();

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
            Q_EMIT programCountChanged();
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
    QSqlQuery programQuery;
    programQuery.prepare(QStringLiteral("SELECT * FROM Programs WHERE channel=:channel ORDER BY start"));
    programQuery.bindValue(QStringLiteral(":channel"), m_id);
    Database::instance().execute(programQuery);
    int programIndex = 0;
    while (programQuery.next()) {
        loadProgram(programIndex);
        index++;
    }

    connect(&Fetcher::instance(), &Fetcher::channelUpdated, this, [this](const QString &id) {
        if (this->id() == id) {
            for (auto &program : m_programs) {
                delete program;
            }
            m_programs.clear();
        }
    });
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

int Channel::programCount() const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (id) FROM Programs where channel=:channel;"));
    query.bindValue(QStringLiteral(":channel"), m_url);
    Database::instance().execute(query);
    if (!query.next()) {
        return -1;
    }
    return query.value(0).toInt();
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
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE Channels SET favorite=:favorite WHERE url=:url;"));
    query.bindValue(QStringLiteral(":favorite"), favorite);
    query.bindValue(QStringLiteral(":url"), m_url);
    Database::instance().execute(query);
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

    // Delete Channel
    query.prepare(QStringLiteral("DELETE FROM Channels WHERE url=:url;"));
    query.bindValue(QStringLiteral(":url"), m_url);
    Database::instance().execute(query);
}

void Channel::loadProgram(int index) const
{
    m_programs[index] = new Program(this, index);
}
