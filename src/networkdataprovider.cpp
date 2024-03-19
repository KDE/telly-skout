// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "networkdataprovider.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>

NetworkDataProvider::NetworkDataProvider(QNetworkAccessManager *nam)
{
    if (nam) {
        m_manager = nam;
    } else {
        m_manager = new QNetworkAccessManager(this);
        m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        m_manager->setStrictTransportSecurityEnabled(true);
        m_manager->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/hsts/"));
    }
}

void NetworkDataProvider::get(const QUrl &url, std::function<void(const QByteArray &)> callback, std::function<void(const Error &)> errorCallback) const
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "telly-skout/0.1");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, callback, errorCallback]() {
        if (reply->error() == QNetworkReply::NoError) {
            callback(reply->readAll());
        } else {
            errorCallback(Error(reply->error(), reply->errorString()));
        }

        reply->deleteLater();
    });
}
