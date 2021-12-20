/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fetcher.h"

#include "database.h"

#include <KLocalizedString>

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QVector>
#include <QtXml>

Fetcher::Fetcher()
{
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    m_manager->setStrictTransportSecurityEnabled(true);
    m_manager->enableStrictTransportSecurityStore(true);
}

void Fetcher::fetchFavorites()
{
#ifdef USE_TVSPIELFILM
    m_tvSpielfilmFetcher.fetchFavorites();
#else
    m_xmlTvSeFetcher.fetchFavorites();
#endif
}

void Fetcher::fetchCountries()
{
#ifdef USE_TVSPIELFILM
    m_tvSpielfilmFetcher.fetchCountries();
#else
    m_xmlTvSeFetcher.fetchCountries();
#endif
}

void Fetcher::fetchCountry(const QString &url, const QString &countryId)
{
    fetchCountry(url, CountryId(countryId));
}

void Fetcher::fetchCountry(const QString &url, const CountryId &countryId)
{
#ifdef USE_TVSPIELFILM
    m_tvSpielfilmFetcher.fetchCountry(url, countryId);
#else
    m_xmlTvSeFetcher.fetchCountry(url, countryId);
#endif
}

void Fetcher::fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url)
{
#ifdef USE_TVSPIELFILM
    m_tvSpielfilmFetcher.fetchProgramDescription(ChannelId(channelId), ProgramId(programId), url);
#else
    m_xmlTvSeFetcher.fetchProgramDescription(ChannelId(channelId), ProgramId(programId), url);
#endif
}

QString Fetcher::image(const QString &url)
{
    QString path = filePath(url);
    if (QFileInfo::exists(path)) {
        return path;
    }

    download(url);

    return QLatin1String("");
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
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/")
        + QString::fromStdString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex().toStdString());
}

QNetworkReply *Fetcher::get(QNetworkRequest &request)
{
    request.setRawHeader("User-Agent", "telly-skout/0.1");
    return m_manager->get(request);
}

//
// TODO: rework
//
void Fetcher::emitStartedFetchingFavorites()
{
    Q_EMIT startedFetchingFavorites();
}

void Fetcher::emitFinishedFetchingFavorites()
{
    Q_EMIT finishedFetchingFavorites();
}

void Fetcher::emitStartedFetchingCountry(const CountryId &id)
{
    Q_EMIT startedFetchingCountry(id);
}

void Fetcher::emitStartedFetchingChannel(const ChannelId &id)
{
    Q_EMIT startedFetchingChannel(id);
}

void Fetcher::emitCountryUpdated(const CountryId &id)
{
    Q_EMIT countryUpdated(id);
}

void Fetcher::emitChannelUpdated(const ChannelId &id)
{
    Q_EMIT channelUpdated(id);
}

void Fetcher::emitCountryDetailsUpdated(const CountryId &id)
{
    Q_EMIT countryDetailsUpdated(id);
}

void Fetcher::emitChannelDetailsUpdated(const ChannelId &id, const QString &image)
{
    Q_EMIT channelDetailsUpdated(id, image);
}

void Fetcher::emitErrorFetching(const Error &error)
{
    Q_EMIT errorFetching(error);
}

void Fetcher::emitErrorFetchingCountry(const CountryId &id, const Error &error)
{
    Q_EMIT errorFetchingCountry(id, error);
}

void Fetcher::emitErrorFetchingChannel(const ChannelId &id, const Error &error)
{
    Q_EMIT errorFetchingChannel(id, error);
}

void Fetcher::emitErrorFetchingProgram(const ProgramId &id, const Error &error)
{
    Q_EMIT errorFetchingProgram(id, error);
}

void Fetcher::emitImageDownloadFinished(const QString &url)
{
    Q_EMIT imageDownloadFinished(url);
}
