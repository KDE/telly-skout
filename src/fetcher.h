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
    void fetchFavorites();
    Q_INVOKABLE void fetchCountries();
    Q_INVOKABLE void fetchCountry(const QString &url, const QString &countryId);
    void fetchCountry(const QString &url, const CountryId &countryId);
    Q_INVOKABLE void fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url);
    Q_INVOKABLE QString image(const QString &url);
    Q_INVOKABLE void download(const QString &url);

private:
    Fetcher();

    QString filePath(const QString &url);
    void removeImage(const QString &url);
    QNetworkReply *get(QNetworkRequest &request);

    QNetworkAccessManager *m_manager;
    std::unique_ptr<FetcherImpl> m_fetcherImpl;

Q_SIGNALS:
    void startedFetchingFavorites();
    void finishedFetchingFavorites();

    void startedFetchingCountry(const CountryId &id);
    void countryUpdated(const CountryId &id);

    void startedFetchingChannel(const ChannelId &id);
    void channelUpdated(const ChannelId &id);
    void channelDetailsUpdated(const ChannelId &id, const QString &image);

    void errorFetching(const Error &error);
    void errorFetchingCountry(const CountryId &id, const Error &error);
    void errorFetchingChannel(const ChannelId &id, const Error &error);
    void errorFetchingProgram(const ProgramId &id, const Error &error);

    void imageDownloadFinished(const QString &url);
};
