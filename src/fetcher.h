// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fetcherimpl.h"
#include "types.h"

#include <QObject>

#include <memory>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QString;

class Fetcher : public QObject
{
    Q_OBJECT
public:
    static Fetcher &instance()
    {
        static Fetcher _instance;
        return _instance;
    }
    Q_INVOKABLE void fetchFavorites();
    Q_INVOKABLE void fetchGroups();
    Q_INVOKABLE void fetchGroup(const QString &url, const QString &groupId);
    void fetchGroup(const QString &url, const GroupId &groupId);
    Q_INVOKABLE void fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url);
    Q_INVOKABLE QString image(const QString &url);

private:
    Fetcher();

    void removeImage(const QString &url);

    std::unique_ptr<FetcherImpl> m_fetcherImpl;

Q_SIGNALS:
    void startedFetchingGroup(const GroupId &id);
    void groupUpdated(const GroupId &id);

    void startedFetchingChannel(const ChannelId &id);
    void channelUpdated(const ChannelId &id);
    void channelDetailsUpdated(const ChannelId &id, const QString &image);

    void errorFetching(const Error &error);
    void errorFetchingGroup(const GroupId &id, const Error &error);
    void errorFetchingChannel(const ChannelId &id, const Error &error);
    void errorFetchingProgram(const ProgramId &id, const Error &error);

    void imageDownloadFinished(const QString &url);
};
