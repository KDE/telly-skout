// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "networkfetcher.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

NetworkFetcher::NetworkFetcher(QNetworkAccessManager *nam)
    : m_provider(nam)
{
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

void NetworkFetcher::downloadImage(const QString &url)
{
    m_provider.get(
        QUrl(url),
        [this, url](QByteArray data) {
            QFile file(imagePath(url));
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.close();
            Q_EMIT imageDownloadFinished(url);
        },
        [url](Error error) {
            qWarning() << "Failed to download image" << url << ":" << error.m_message;
        });
}
