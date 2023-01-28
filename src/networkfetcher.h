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

    void fetchGroups(std::function<void(const QVector<GroupData> &)> callback = nullptr,
                     std::function<void(const Error &)> errorCallback = nullptr) override = 0;
    void fetchGroup(const QString &url,
                    const GroupId &groupId,
                    std::function<void(const QList<ChannelData> &)> callback = nullptr,
                    std::function<void(const Error &)> errorCallback = nullptr) override = 0;
    void fetchProgram(const ChannelId &channelId,
                      std::function<void(const QVector<ProgramData> &)> callback = nullptr,
                      std::function<void(const Error &)> errorCallback = nullptr) override = 0;
    void fetchProgramDescription(const ChannelId &channelId,
                                 const ProgramId &programId,
                                 const QString &url,
                                 std::function<void(const QString &)> callback = nullptr,
                                 std::function<void(const Error &)> errorCallback = nullptr) override = 0;
    QString image(const QString &url, std::function<void()> callback = nullptr, std::function<void(const Error &)> errorCallback = nullptr) override;
    QString imagePath(const QString &url) override;

protected:
    const NetworkDataProvider m_provider;

private:
    void downloadImage(const QString &url, std::function<void()> callback, std::function<void(const Error &)> errorCallback);
};
