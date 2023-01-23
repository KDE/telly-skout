// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "localdataprovider.h"

#include <QFile>

void LocalDataProvider::get(const QUrl &url, std::function<void(const QByteArray &)> callback, std::function<void(Error)> errorCallback) const
{
    QFile file(url.toLocalFile());
    if (file.open(QIODevice::ReadOnly)) {
        callback(file.readAll());
    } else {
        errorCallback(Error("Could not open" + url.toDisplayString()));
    }
    file.close();
}
