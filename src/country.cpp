/*
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "country.h"

#include "channelsmodel.h"
#include "database.h"
#include "fetcher.h"

#include <QDebug>
#include <QSqlQuery>

Country::Country(CountryData data)
    : QObject(nullptr)
    , m_data(data)
{
    m_errorId = 0;
    m_errorString = QLatin1String("");

    connect(&Fetcher::instance(), &Fetcher::startedFetchingCountry, this, [this](const CountryId &id) {
        if (id == m_data.m_id) {
            setRefreshing(true);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::countryUpdated, this, [this](const CountryId &id) {
        if (id == m_data.m_id) {
            setRefreshing(false);
            Q_EMIT channelCountChanged();
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

    m_channels = new ChannelsModel(this);
}

Country::~Country()
{
}

QString Country::id() const
{
    return m_data.m_id.value();
}

QString Country::name() const
{
    return m_data.m_name;
}

QString Country::url() const
{
    return m_data.m_url;
}

int Country::channelCount() const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (id) FROM CountryChannels where country=:country;"));
    query.bindValue(QStringLiteral(":country"), m_data.m_id.value());
    Database::instance().execute(query);
    if (!query.next()) {
        return -1;
    }
    return query.value(0).toInt();
}

bool Country::refreshing() const
{
    return m_refreshing;
}

int Country::errorId() const
{
    return m_errorId;
}

QString Country::errorString() const
{
    return m_errorString;
}

void Country::setName(const QString &name)
{
    m_data.m_name = name;
    Q_EMIT nameChanged(m_data.m_name);
}

void Country::setRefreshing(bool refreshing)
{
    m_refreshing = refreshing;
    Q_EMIT refreshingChanged(m_refreshing);
}

void Country::setErrorId(int errorId)
{
    m_errorId = errorId;
    Q_EMIT errorIdChanged(m_errorId);
}

void Country::setErrorString(const QString &errorString)
{
    m_errorString = errorString;
    Q_EMIT errorStringChanged(m_errorString);
}
