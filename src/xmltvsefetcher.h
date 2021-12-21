#pragma once

#include "networkfetcher.h"

class QDomElement;
class QDomNode;

class XmlTvSeFetcher : public NetworkFetcher
{
    Q_OBJECT
public:
    XmlTvSeFetcher();
    virtual ~XmlTvSeFetcher() = default;

    void fetchCountries() override;
    void fetchCountry(const QString &url, const CountryId &countryId) override;
    void fetchProgram(const ChannelId &channelId) override;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override;

private:
    void fetchChannel(const ChannelId &channelId, const QString &name, const CountryId &countryId);
    void processCountry(const QDomElement &country);
    void processChannel(const QDomElement &channel);
    void processProgram(const QDomNode &program);
};
