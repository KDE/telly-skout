// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "channel.h"

#include "database.h"
#include "fetcher.h"
#include "program.h"
#include "programfactory.h"
#include "programsmodel.h"
#include "types.h"

#include <QDebug>

Channel::Channel(const ChannelData &data, bool favorite, const QVector<QString> &groupIds, ProgramFactory &programFactory)
    : QObject(nullptr)
    , m_data(data)
    , m_favorite(favorite)
    , m_groups(groupIds)
{
    connect(&Fetcher::instance(), &Fetcher::startedFetchingChannel, this, [this](const ChannelId &id) {
        if (id == m_data.m_id) {
            setRefreshing(true);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::channelUpdated, this, [this](const ChannelId &id) {
        if (id == m_data.m_id) {
            setRefreshing(false);
            Q_EMIT programChanged();
            m_error.reset();
        }
    });
    connect(&Fetcher::instance(), &Fetcher::errorFetchingChannel, this, [this](const ChannelId &id, const Error &error) {
        if (id == m_data.m_id) {
            setError(error);
            setRefreshing(false);
        }
    });
    connect(&Fetcher::instance(), &Fetcher::imageDownloadFinished, this, [this](const QString &url) {
        if (url == m_data.m_image) {
            Q_EMIT imageChanged(url);
        }
    });

    // programs
    m_programsModel = new ProgramsModel(this, programFactory);
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

QVector<QString> Channel::groups() const
{
    return m_groups;
}

bool Channel::refreshing() const
{
    return m_refreshing;
}

int Channel::errorId() const
{
    return m_error.m_id;
}

QString Channel::errorString() const
{
    return m_error.m_message;
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

void Channel::setGroups(const QVector<QString> &groups)
{
    m_groups = groups;
    Q_EMIT groupsChanged(m_groups);
}

void Channel::setRefreshing(bool refreshing)
{
    m_refreshing = refreshing;
    Q_EMIT refreshingChanged(m_refreshing);
}

void Channel::setError(const Error &error)
{
    m_error = error;
    Q_EMIT errorIdChanged(m_error.m_id);
    Q_EMIT errorStringChanged(m_error.m_message);
}
