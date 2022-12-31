// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "networkfetcher.h"

#include "programdata.h"

class TvSpielfilmFetcher : public NetworkFetcher
{
    Q_OBJECT
public:
    TvSpielfilmFetcher(QNetworkAccessManager *nam = nullptr);
    virtual ~TvSpielfilmFetcher() = default;

    void fetchGroups() override;
    void fetchGroup(const QString &url, const GroupId &groupId) override;
    void fetchProgram(const ChannelId &channelId) override;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override;

private:
    void fetchChannel(const ChannelId &channelId, const QString &name, const GroupId &group);
    void fetchProgram(const ChannelId &channelId, const QString &url, QVector<ProgramData> &programs);
    QVector<ProgramData> processChannel(const QString &infoTable, const QString &url, const ChannelId &channelId);
    ProgramData processProgram(const QRegularExpressionMatch &programMatch, const QString &url, const ChannelId &channelId, bool isLast);
    void processDescription(const QString &descriptionPage, const QString &url, const ProgramId &programId);
};
