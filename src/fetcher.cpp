// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "fetcher.h"

#include "TellySkoutSettings.h"
#include "database.h"
#include "tvspielfilmfetcher.h"
#include "xmltvfetcher.h"

#include <KLocalizedString>

#include <QAtomicInteger>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>

Fetcher::Fetcher()
    : m_favoritesPercentage(0)
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
    static QAtomicInteger<qsizetype> channelCounter = 0; // reference counter to determine when all channels have been fetched

    qDebug() << "Starting to fetch favorites";

    const QVector<ChannelId> favoriteChannels = Database::instance().favorites();

    // do nothing if favorites are already being fetched (e.g. triggered from main() and from QML)
    if (!channelCounter.testAndSetOrdered(0, favoriteChannels.size())) {
        qDebug() << "Favorites are already being fetched, do nothing";
        return;
    }

    setFavoritesPercentage(0);

    if (favoriteChannels.empty()) {
        setFavoritesPercentage(100);
        return;
    }

    for (const ChannelId &channelId : favoriteChannels) {
        Q_EMIT startedFetchingChannel(channelId);
        m_fetcherImpl->fetchProgram(
            channelId,
            [this, channelId, favoriteChannels](const QVector<ProgramData> &programs) {
                if (!programs.empty()) {
                    Database::instance().addPrograms(programs);
                    Q_EMIT channelUpdated(channelId);
                }
                Q_EMIT finishedFetchingChannel(channelId);

                channelCounter.deref();
                setFavoritesPercentage((100 - static_cast<unsigned int>((channelCounter * 100) / favoriteChannels.size())));
            },
            [this, channelId, favoriteChannels](const Error &error) {
                Q_EMIT errorFetchingChannel(channelId, error);

                channelCounter.deref();
                setFavoritesPercentage((100 - static_cast<unsigned int>((channelCounter * 100) / favoriteChannels.size())));
            });
    }
}

void Fetcher::fetchGroups()
{
    m_fetcherImpl->fetchGroups([](const QVector<GroupData> &groups) {
        Database::instance().addGroups(groups);
    });
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

void Fetcher::fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url)
{
    m_fetcherImpl->fetchProgramDescription(channelId, programId, url, [this, channelId, programId](const QString &description) {
        Database::instance().updateProgramDescription(programId, description);
        Q_EMIT programUpdated(programId);
    });
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

void Fetcher::setImpl(std::unique_ptr<FetcherImpl> fetcherImpl)
{
    m_fetcherImpl = std::move(fetcherImpl);
}

unsigned int Fetcher::favoritesPercentage()
{
    return m_favoritesPercentage;
}

void Fetcher::setFavoritesPercentage(unsigned int percentage)
{
    m_favoritesPercentage = percentage;
    Q_EMIT favoritesPercentageChanged(m_favoritesPercentage);
}

void Fetcher::removeImage(const QString &url)
{
    qDebug() << "Remove image: " << m_fetcherImpl->imagePath(url);
    QFile(m_fetcherImpl->imagePath(url)).remove();
}

#include "moc_fetcher.cpp"
