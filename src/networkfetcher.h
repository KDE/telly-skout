// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "fetcherimpl.h"

#include "networkdataprovider.h"

class NetworkFetcher : public FetcherImpl
{
    Q_OBJECT
public:
    NetworkFetcher(QNetworkAccessManager *nam = nullptr);
    virtual ~NetworkFetcher() = default;

    void fetchGroups() override = 0;
    void fetchGroup(const QString &url, const GroupId &groupId) override = 0;
    void fetchProgram(const ChannelId &channelId) override = 0;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override = 0;
    QString image(const QString &url) override;
    QString imagePath(const QString &url) override;

protected:
    void downloadImage(const QString &url);
    const NetworkDataProvider m_provider;
};
