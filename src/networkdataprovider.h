// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "dataprovider.h"

class QNetworkAccessManager;

class NetworkDataProvider : DataProvider
{
public:
    explicit NetworkDataProvider(QNetworkAccessManager *nam = nullptr);
    virtual ~NetworkDataProvider() = default;

    void get(const QUrl &url, std::function<void(const QByteArray &)> callback, std::function<void(Error)> errorCallback = nullptr) const override;

private:
    QNetworkAccessManager *m_manager;
};
