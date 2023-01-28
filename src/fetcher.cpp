// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "fetcher.h"

#include "TellySkoutSettings.h"
#include "database.h"
#include "tvspielfilmfetcher.h"
#include "xmltvfetcher.h"

#include <KLocalizedString>

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>

Fetcher::Fetcher()
{
    const TellySkoutSettings::EnumFetcher::type fetcherType = static_cast<TellySkoutSettings::EnumFetcher::type>(TellySkoutSettings::fetcher());

    switch (fetcherType) {
    case TellySkoutSettings::EnumFetcher::TVSpielfilm:
        m_fetcherImpl.reset(new TvSpielfilmFetcher);
        break;
    case TellySkoutSettings::EnumFetcher::XMLTV:
        m_fetcherImpl.reset(new XmltvFetcher);
        break;
    case TellySkoutSettings::EnumFetcher::COUNT:
        qDebug() << "Invalid Fetcher type!";
        assert(false);
    }
}

void Fetcher::fetchFavorites()
{
    qDebug() << "Starting to fetch favorites";

    const QVector<ChannelId> favoriteChannels = Database::instance().favorites();
    for (const ChannelId &channelId : favoriteChannels) {
        Q_EMIT startedFetchingChannel(channelId);
        m_fetcherImpl->fetchProgram(
            channelId,
            [this, channelId](const QVector<ProgramData> &programs) {
                Database::instance().addPrograms(programs);
                Q_EMIT channelUpdated(channelId);
            },
            [this, channelId](const Error &error) {
                Q_EMIT errorFetchingChannel(channelId, error);
            });
    }
}

void Fetcher::fetchGroups()
{
    m_fetcherImpl->fetchGroups([](const QVector<GroupData> &groups) {
        Database::instance().addGroups(groups);
    });
}

void Fetcher::fetchGroup(const QString &url, const QString &groupId)
{
    fetchGroup(url, GroupId(groupId));
}

void Fetcher::fetchGroup(const QString &url, const GroupId &groupId)
{
    Q_EMIT startedFetchingGroup(groupId);
    m_fetcherImpl->fetchGroup(
        url,
        groupId,
        [this, groupId](const QList<ChannelData> &channels) {
            Database::instance().addChannels(channels, groupId);
            Q_EMIT groupUpdated(groupId);
        },
        [this, groupId](const Error &error) {
            Q_EMIT errorFetchingGroup(groupId, error);
        });
}

void Fetcher::fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url)
{
    fetchProgramDescription(ChannelId(channelId), ProgramId(programId), url);
}

void Fetcher::fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url)
{
    m_fetcherImpl->fetchProgramDescription(ChannelId(channelId), ProgramId(programId), url, [this, channelId, programId](const QString &description) {
        Database::instance().updateProgramDescription(programId, description);
        Q_EMIT channelUpdated(channelId);
    }); // TODO: separate signal
}

QString Fetcher::image(const QString &url)
{
    return m_fetcherImpl->image(
        url,
        [this, url]() {
            Q_EMIT imageDownloadFinished(url);
        },
        [this, url](const Error &error) {
            Q_EMIT errorDownloadingImage(url, error);
        });
}

void Fetcher::removeImage(const QString &url)
{
    qDebug() << "Remove image: " << m_fetcherImpl->imagePath(url);
    QFile(m_fetcherImpl->imagePath(url)).remove();
}
