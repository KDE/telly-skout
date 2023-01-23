// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "types.h"

#include <QByteArray>
#include <QObject>
#include <QUrl>

#include <functional>

class DataProvider : public QObject
{
    Q_OBJECT
public:
    virtual ~DataProvider() = default;

    virtual void get(const QUrl &url, std::function<void(const QByteArray &)> callback, std::function<void(Error)> errorCallback = nullptr) const = 0;
};
