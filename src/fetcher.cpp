// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "fetcher.h"

#include "TellySkoutSettings.h"
#include "database.h"
#include "tvspielfilmfetcher.h"
#include "xmltvfetcher.h"

#include <KLocalizedString>

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>

Fetcher::Fetcher()
{
    const TellySkoutSettings settings;
    const TellySkoutSettings::EnumFetcher::type fetcherType = static_cast<TellySkoutSettings::EnumFetcher::type>(settings.fetcher());

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

    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    m_manager->setStrictTransportSecurityEnabled(true);
    m_manager->enableStrictTransportSecurityStore(true);

    connect(m_fetcherImpl.get(), &FetcherImpl::startedFetchingGroup, this, [this](const GroupId &id) {
        Q_EMIT startedFetchingGroup(id);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::groupUpdated, this, [this](const GroupId &id) {
        Q_EMIT groupUpdated(id);
    });

    connect(m_fetcherImpl.get(), &FetcherImpl::startedFetchingChannel, this, [this](const ChannelId &id) {
        Q_EMIT startedFetchingChannel(id);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::channelUpdated, this, [this](const ChannelId &id) {
        Q_EMIT channelUpdated(id);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::channelDetailsUpdated, this, [this](const ChannelId &id, const QString &image) {
        Q_EMIT channelDetailsUpdated(id, image);
    });

    connect(m_fetcherImpl.get(), &FetcherImpl::errorFetching, this, [this](const Error &error) {
        Q_EMIT errorFetching(error);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::errorFetchingGroup, this, [this](const GroupId &id, const Error &error) {
        Q_EMIT errorFetchingGroup(id, error);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::errorFetchingChannel, this, [this](const ChannelId &id, const Error &error) {
        Q_EMIT errorFetchingChannel(id, error);
    });
    connect(m_fetcherImpl.get(), &FetcherImpl::errorFetchingProgram, this, [this](const ProgramId &id, const Error &error) {
        Q_EMIT errorFetchingProgram(id, error);
    });
}

void Fetcher::fetchFavorites()
{
    qDebug() << "Starting to fetch favorites";

    const QVector<ChannelId> favoriteChannels = Database::instance().favorites();
    for (int i = 0; i < favoriteChannels.length(); i++) {
        m_fetcherImpl->fetchProgram(favoriteChannels.at(i));
    }
}

void Fetcher::fetchGroups()
{
    m_fetcherImpl->fetchGroups();
}

void Fetcher::fetchGroup(const QString &url, const QString &groupId)
{
    fetchGroup(url, GroupId(groupId));
}

void Fetcher::fetchGroup(const QString &url, const GroupId &groupId)
{
    m_fetcherImpl->fetchGroup(url, groupId);
}

void Fetcher::fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url)
{
    m_fetcherImpl->fetchProgramDescription(ChannelId(channelId), ProgramId(programId), url);
}

QString Fetcher::image(const QString &url)
{
    QString path = filePath(url);
    if (QFileInfo::exists(path)) {
        return path;
    }

    download(url);

    return "";
}

void Fetcher::download(const QString &url)
{
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QFile file(filePath(url));
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.close();
        }
        Q_EMIT imageDownloadFinished(url);

        delete reply;
    });
}

void Fetcher::removeImage(const QString &url)
{
    qDebug() << "Remove image: " << filePath(url);
    QFile(filePath(url)).remove();
}

QString Fetcher::filePath(const QString &url)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/") + QUrl(url).fileName();
}

QNetworkReply *Fetcher::get(QNetworkRequest &request)
{
    request.setRawHeader("User-Agent", "telly-skout/0.1");
    return m_manager->get(request);
}
