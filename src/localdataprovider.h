// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "dataprovider.h"

class LocalDataProvider : DataProvider
{
public:
    virtual ~LocalDataProvider() = default;

    void get(const QUrl &url, std::function<void(const QByteArray &)> callback, std::function<void(const Error &)> errorCallback = nullptr) const override;
};
