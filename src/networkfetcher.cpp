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

QString NetworkFetcher::image(const QString &url, std::function<void()> callback, std::function<void(const Error &)> errorCallback)
{
    QString path = imagePath(url);
    if (QFileInfo::exists(path)) {
        return path;
    }

    downloadImage(url, callback, errorCallback);

    return QStringLiteral("");
}

QString NetworkFetcher::imagePath(const QString &url)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/") + QUrl(url).fileName();
}

void NetworkFetcher::downloadImage(const QString &url, std::function<void()> callback, std::function<void(const Error &)> errorCallback)
{
    m_provider.get(
        QUrl(url),
        [this, url, callback](QByteArray data) {
            QFile file(imagePath(url));
            const bool success = file.open(QIODevice::WriteOnly);

            if (!success) {
                qWarning() << "Failed to open image" << url << ".";
                return;
            }

            file.write(data);
            file.close();

            if (callback) {
                callback();
            }
        },
        [url, errorCallback](const Error &error) {
            qWarning() << "Failed to download image" << url << ":" << error.m_message;

            if (errorCallback) {
                errorCallback(error);
            }
        });
}

#include "moc_networkfetcher.cpp"
