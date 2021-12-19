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
    m_tvSpielfilmFetcher.fetchFavorites();
}

void Fetcher::fetchCountries()
{
    m_tvSpielfilmFetcher.fetchCountries();
}

void Fetcher::fetchCountry(const QString &url, const QString &countryId)
{
    m_tvSpielfilmFetcher.fetchCountry(url, countryId);
}

void Fetcher::fetchChannel(const QString &channelId, const QString &name, const QString &country)
{
    m_tvSpielfilmFetcher.fetchChannel(channelId, name, country);
}

void Fetcher::fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url)
{
    m_tvSpielfilmFetcher.fetchProgramDescription(channelId, programId, url);
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

void Fetcher::emitStartedFetchingCountry(const QString &id)
{
    Q_EMIT startedFetchingCountry(id);
}

void Fetcher::emitStartedFetchingChannel(const QString &id)
{
    Q_EMIT startedFetchingChannel(id);
}

void Fetcher::emitCountryUpdated(const QString &id)
{
    Q_EMIT countryUpdated(id);
}

void Fetcher::emitChannelUpdated(const QString &id)
{
    Q_EMIT channelUpdated(id);
}

void Fetcher::emitCountryDetailsUpdated(const QString &id)
{
    Q_EMIT countryDetailsUpdated(id);
}

void Fetcher::emitChannelDetailsUpdated(const QString &id, const QString &image)
{
    Q_EMIT channelDetailsUpdated(id, image);
}

void Fetcher::emitError(const QString &id, int errorId, const QString &errorString)
{
    Q_EMIT error(id, errorId, errorString);
}

void Fetcher::emitImageDownloadFinished(const QString &url)
{
    Q_EMIT imageDownloadFinished(url);
}
