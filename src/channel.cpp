/*
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QVariant>

#include "channel.h"
#include "database.h"
#include "fetcher.h"
#include "programsmodel.h"

Channel::Channel(int index)
    : QObject(nullptr)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT * FROM Channels LIMIT 1 OFFSET :index;"));
    query.bindValue(QStringLiteral(":index"), index);
    Database::instance().execute(query);
    if (!query.next()) {
        qWarning() << "Failed to load channel" << index;
    }

    /*QSqlQuery countryQuery;
    countryQuery.prepare(QStringLiteral("SELECT * FROM Countries WHERE id='' AND channel=:channel"));
    countryQuery.bindValue(QStringLiteral(":channel"), query.value(QStringLiteral("url")).toString());
    Database::instance().execute(countryQuery);
    while (countryQuery.next()) {
        m_countries += new Country(countryQuery.value(QStringLiteral("name")).toString(), countryQuery.value(QStringLiteral("url")).toString(), nullptr);
    }*/

    m_name = query.value(QStringLiteral("name")).toString();
    m_url = query.value(QStringLiteral("url")).toString();
    m_image = query.value(QStringLiteral("image")).toString();
    m_notify = query.value(QStringLiteral("notify")).toBool();
    m_favorite = query.value(QStringLiteral("favorite")).toBool();

    m_errorId = 0;
    m_errorString = QLatin1String("");

    connect(&Fetcher::instance(), &Fetcher::startedFetchingChannel, this, [this](const QString &url) {
        if (url == m_url) {
            setRefreshing(true);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::channelUpdated, this, [this](const QString &url) {
        if (url == m_url) {
            setRefreshing(false);
            Q_EMIT programCountChanged();
            setErrorId(0);
            setErrorString(QLatin1String(""));
        }
    });
    connect(&Fetcher::instance(), &Fetcher::error, this, [this](const QString &url, int errorId, const QString &errorString) {
        if (url == m_url) {
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

    m_programs = new ProgramsModel(this);
}

Channel::~Channel()
{
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

QVector<Country *> Channel::countries() const
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

void Channel::setCountries(const QVector<Country *> &countries)
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
    Fetcher::instance().fetchChannel(m_url, m_url); // TODO: url -> ID
}

void Channel::setAsFavorite()
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE Channels SET favorite=TRUE WHERE url=:url;"));
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

    // TODO Delete Enclosures

    // Delete Channel
    query.prepare(QStringLiteral("DELETE FROM Channels WHERE url=:url;"));
    query.bindValue(QStringLiteral(":url"), m_url);
    Database::instance().execute(query);
}
