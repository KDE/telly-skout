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

    virtual void fetchCountries() = 0;
    virtual void fetchCountry(const QString &url, const CountryId &countryId) = 0;
    virtual void fetchProgram(const ChannelId &channelId) = 0;
    virtual void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) = 0;

Q_SIGNALS:
    void startedFetchingCountry(const CountryId &id);
    void countryUpdated(const CountryId &id);

    void startedFetchingChannel(const ChannelId &id);
    void channelUpdated(const ChannelId &id);
    void channelDetailsUpdated(const ChannelId &id, const QString &image);

    void errorFetching(const Error &error);
    void errorFetchingCountry(const CountryId &id, const Error &error);
    void errorFetchingChannel(const ChannelId &id, const Error &error);
    void errorFetchingProgram(const ProgramId &id, const Error &error);
};
