// SPDX-FileCopyrightText: none
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

protected:
    QNetworkReply *get(QNetworkRequest &request);

private:
    QNetworkAccessManager *m_manager;
};
