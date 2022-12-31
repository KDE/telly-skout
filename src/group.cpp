// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: GPL-3.0-only

#include "group.h"

#include "channelsmodel.h"
#include "fetcher.h"

#include <QDebug>

Group::Group(const GroupData &data)
    : QObject(nullptr)
    , m_data(data)
{
    connect(&Fetcher::instance(), &Fetcher::startedFetchingGroup, this, [this](const GroupId &id) {
        if (id == m_data.m_id) {
            setRefreshing(true);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::groupUpdated, this, [this](const GroupId &id) {
        if (id == m_data.m_id) {
            setRefreshing(false);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::errorFetchingGroup, this, [this](const GroupId &id, const Error &error) {
        if (id == m_data.m_id) {
            setError(error);
            setRefreshing(false);
        }
    });

    m_channels = new ChannelsModel(this);
}

Group::~Group()
{
}

QString Group::id() const
{
    return m_data.m_id.value();
}

QString Group::name() const
{
    return m_data.m_name;
}

QString Group::url() const
{
    return m_data.m_url;
}

bool Group::refreshing() const
{
    return m_refreshing;
}

int Group::errorId() const
{
    return m_error.m_id;
}

QString Group::errorString() const
{
    return m_error.m_message;
}

void Group::setName(const QString &name)
{
    m_data.m_name = name;
    Q_EMIT nameChanged(m_data.m_name);
}

void Group::setRefreshing(bool refreshing)
{
    m_refreshing = refreshing;
    Q_EMIT refreshingChanged(m_refreshing);
}

void Group::setError(const Error &error)
{
    m_error = error;
    Q_EMIT errorIdChanged(m_error.m_id);
    Q_EMIT errorStringChanged(m_error.m_message);
}
