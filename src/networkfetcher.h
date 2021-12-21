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

    void fetchFavorites() override = 0;
    void fetchCountries() override = 0;
    void fetchCountry(const QString &url, const CountryId &countryId) override = 0;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override = 0;

protected:
    QNetworkReply *get(QNetworkRequest &request);

private:
    QNetworkAccessManager *m_manager;
};
