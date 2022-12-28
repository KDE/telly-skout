// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "networkfetcher.h"

#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>

NetworkFetcher::NetworkFetcher()
{
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    m_manager->setStrictTransportSecurityEnabled(true);
    m_manager->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/hsts/"));
}

QString NetworkFetcher::image(const QString &url)
{
    QString path = imagePath(url);
    if (QFileInfo::exists(path)) {
        return path;
    }

    downloadImage(url);

    return "";
}

QString NetworkFetcher::imagePath(const QString &url)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/") + QUrl(url).fileName();
}

QNetworkReply *NetworkFetcher::get(QNetworkRequest &request)
{
    request.setRawHeader("User-Agent", "telly-skout/0.1");
    return m_manager->get(request);
}

void NetworkFetcher::downloadImage(const QString &url)
{
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QFile file(imagePath(url));
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.close();
        }
        Q_EMIT imageDownloadFinished(url);

        reply->deleteLater();
    });
}
