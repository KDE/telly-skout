// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "channeldata.h"
#include "groupdata.h"
#include "programdata.h"
#include "types.h"

#include <QList>
#include <QVector>

#include <functional>

class QString;

class FetcherImpl : public QObject
{
    Q_OBJECT
public:
    virtual ~FetcherImpl() = default;

    virtual void fetchGroups(std::function<void(const QVector<GroupData> &)> callback = nullptr,
                             std::function<void(const Error &)> errorCallback = nullptr) = 0;
    virtual void fetchGroup(const QString &url,
                            const GroupId &groupId,
                            std::function<void(const QList<ChannelData> &)> callback = nullptr,
                            std::function<void(const Error &)> errorCallback = nullptr) = 0;
    virtual void fetchProgram(const ChannelId &channelId,
                              std::function<void(const QVector<ProgramData> &)> callback = nullptr,
                              std::function<void(const Error &)> errorCallback = nullptr) = 0;
    virtual void fetchProgramDescription(const ChannelId &channelId,
                                         const ProgramId &programId,
                                         const QString &url,
                                         std::function<void(const QString &)> callback = nullptr,
                                         std::function<void(const Error &)> errorCallback = nullptr) = 0;
    virtual QString image(const QString &url, std::function<void()> callback = nullptr, std::function<void(const Error &)> errorCallback = nullptr) = 0;
    virtual QString imagePath(const QString &url) = 0;
};
