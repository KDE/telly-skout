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
#include "types.h"

#include <QDebug>
#include <QSqlQuery>

Channel::Channel(const ChannelData &data)
    : QObject(nullptr)
{
    m_data = data;

    // TODO: use ChannelFactory
    QSqlQuery favoriteQuery;
    favoriteQuery.prepare(QStringLiteral("SELECT id FROM Favorites WHERE channel=:channel"));
    favoriteQuery.bindValue(QStringLiteral(":channel"), m_data.m_id.value());
    Database::instance().execute(favoriteQuery);
    m_favorite = favoriteQuery.next();

    QSqlQuery countryQuery;
    countryQuery.prepare(QStringLiteral("SELECT country FROM CountryChannels WHERE channel=:channel"));
    countryQuery.bindValue(QStringLiteral(":channel"), m_data.m_id.value());
    Database::instance().execute(countryQuery);
    while (countryQuery.next()) {
        m_countries.push_back(countryQuery.value(QStringLiteral("country")).toString());
    }

    m_errorId = 0;
    m_errorString = QLatin1String("");

    connect(&Fetcher::instance(), &Fetcher::startedFetchingChannel, this, [this](const ChannelId &id) {
        if (id == m_data.m_id) {
            setRefreshing(true);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::channelUpdated, this, [this](const ChannelId &id) {
        if (id == m_data.m_id) {
            setRefreshing(false);
            Q_EMIT programChanged();
            setErrorId(0);
            setErrorString(QLatin1String(""));
        }
    });
    connect(&Fetcher::instance(), &Fetcher::error, this, [this](const QString &id, int errorId, const QString &errorString) {
        if (id == m_data.m_id.value()) {
            setErrorId(errorId);
            setErrorString(errorString);
            setRefreshing(false);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::imageDownloadFinished, this, [this](const QString &url) {
        if (url == m_data.m_image) {
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
    return m_data.m_id.value();
}

QString Channel::url() const
{
    return m_data.m_url;
}

QString Channel::name() const
{
    return m_data.m_name;
}

QString Channel::image() const
{
    return m_data.m_image;
}

bool Channel::favorite() const
{
    return m_favorite;
}

QVector<QString> Channel::countries() const
{
    return m_countries;
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
    m_data.m_name = name;
    Q_EMIT nameChanged(m_data.m_name);
}

void Channel::setImage(const QString &image)
{
    m_data.m_image = image;
    Q_EMIT imageChanged(m_data.m_image);
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
