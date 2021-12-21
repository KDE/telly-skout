#pragma once

#include "networkfetcher.h"

#include "programdata.h"

class TvSpielfilmFetcher : public NetworkFetcher
{
    Q_OBJECT
public:
    TvSpielfilmFetcher();
    virtual ~TvSpielfilmFetcher() = default;

    void fetchCountries() override;
    void fetchCountry(const QString &url, const CountryId &countryId) override;
    void fetchProgram(const ChannelId &channelId) override;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override;

private:
    void fetchChannel(const ChannelId &channelId, const QString &name, const CountryId &country);
    void fetchProgram(const ChannelId &channelId, const QString &url);
    void processChannel(const QString &infoTable, const QString &url, const ChannelId &channelId);
    ProgramData processProgram(const QRegularExpressionMatch &programMatch, const QString &url, const ChannelId &channelId, bool isLast);
    void processDescription(const QString &descriptionPage, const QString &url, const ProgramId &programId);
};
