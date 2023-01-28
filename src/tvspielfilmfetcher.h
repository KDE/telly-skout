// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "networkfetcher.h"

#include "programdata.h"

#include <QMap>

class QDate;

class TvSpielfilmFetcher : public NetworkFetcher
{
    Q_OBJECT
public:
    explicit TvSpielfilmFetcher(QNetworkAccessManager *nam = nullptr);
    virtual ~TvSpielfilmFetcher() = default;

    void fetchGroups(std::function<void(const QVector<GroupData> &)> callback = nullptr, std::function<void(const Error &)> errorCallback = nullptr) override;
    void fetchGroup(const QString &url,
                    const GroupId &groupId,
                    std::function<void(const QList<ChannelData> &)> callback = nullptr,
                    std::function<void(const Error &)> errorCallback = nullptr) override;
    void fetchProgram(const ChannelId &channelId,
                      std::function<void(const QVector<ProgramData> &)> callback = nullptr,
                      std::function<void(const Error &)> errorCallback = nullptr) override;
    void fetchProgramDescription(const ChannelId &channelId,
                                 const ProgramId &programId,
                                 const QString &url,
                                 std::function<void(const QString &)> callback = nullptr,
                                 std::function<void(const Error &)> errorCallback = nullptr) override;

private:
    void fetchChannel(const ChannelId &channelId, const QString &name, QMap<ChannelId, ChannelData> &channels);
    void fetchProgram(const ChannelId &channelId,
                      const QDate &date,
                      unsigned int page,
                      QVector<ProgramData> &programs,
                      std::function<void(const QVector<ProgramData> &)> callback = nullptr,
                      std::function<void(const Error &)> errorCallback = nullptr);
    QVector<ProgramData> processChannel(const QString &infoTable, const QString &url, const ChannelId &channelId);
    ProgramData processProgram(const QRegularExpressionMatch &programMatch, const QString &url, const ChannelId &channelId, bool isLast);
    QString processDescription(const QString &descriptionPage, const QString &url);
    bool programExists(const ChannelId &channelId, const QDate &date);
};
