// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fetcherimpl.h"

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class NetworkFetcher : public FetcherImpl
{
    Q_OBJECT
public:
    NetworkFetcher();
    virtual ~NetworkFetcher() = default;

    void fetchGroups() override = 0;
    void fetchGroup(const QString &url, const GroupId &groupId) override = 0;
    void fetchProgram(const ChannelId &channelId) override = 0;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override = 0;
    QString image(const QString &url) override;
    QString imagePath(const QString &url) override;

protected:
    QNetworkReply *get(QNetworkRequest &request);
    void downloadImage(const QString &url);

private:
    QNetworkAccessManager *m_manager;
};
