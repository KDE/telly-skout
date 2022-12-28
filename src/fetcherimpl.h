// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include "types.h"

class QString;

class FetcherImpl : public QObject
{
    Q_OBJECT
public:
    virtual ~FetcherImpl() = default;

    virtual void fetchGroups() = 0;
    virtual void fetchGroup(const QString &url, const GroupId &groupId) = 0;
    virtual void fetchProgram(const ChannelId &channelId) = 0;
    virtual void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) = 0;
    virtual QString image(const QString &url) = 0;
    virtual QString imagePath(const QString &url) = 0;

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
