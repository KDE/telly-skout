#pragma once

#include "programdata.h"
#include "tvspielfilmfetcher.h"
#include "types.h"
#include "xmltvsefetcher.h"

#include <QObject>

class QDomElement;
class QDomNode;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QString;

#define USE_TVSPIELFILM

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
    Q_INVOKABLE void fetchCountries();
    Q_INVOKABLE void fetchCountry(const QString &url, const QString &countryId); // TODO type safe IDs in qml
    void fetchCountry(const QString &url, const CountryId &countryId);
    Q_INVOKABLE void fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url); // TODO type safe IDs in qml
    Q_INVOKABLE QString image(const QString &url);
    Q_INVOKABLE void download(const QString &url);

private:
    Fetcher();

    QString filePath(const QString &url);
    void removeImage(const QString &url);
    QNetworkReply *get(QNetworkRequest &request);

    QNetworkAccessManager *m_manager;
#ifdef USE_TVSPIELFILM
    TvSpielfilmFetcher m_tvSpielfilmFetcher;
#else
    XmlTvSeFetcher m_xmlTvSeFetcher;
#endif

    // TODO: rework
    friend class TvSpielfilmFetcher;
    friend class XmlTvSeFetcher;
    void emitStartedFetchingFavorites();
    void emitFinishedFetchingFavorites();
    void emitStartedFetchingCountry(const CountryId &id); // TODO: emit
    void emitStartedFetchingChannel(const ChannelId &id);
    void emitCountryUpdated(const CountryId &id);
    void emitChannelUpdated(const ChannelId &id);
    void emitCountryDetailsUpdated(const CountryId &id); // TODO: emit
    void emitChannelDetailsUpdated(const ChannelId &id, const QString &image);
    void emitErrorFetching(const Error &error);
    void emitErrorFetchingCountry(const CountryId &id, const Error &error);
    void emitErrorFetchingChannel(const ChannelId &id, const Error &error);
    void emitErrorFetchingProgram(const ProgramId &id, const Error &error);
    void emitImageDownloadFinished(const QString &url);

Q_SIGNALS:
    void startedFetchingFavorites();
    void finishedFetchingFavorites();
    void startedFetchingCountry(const CountryId &id); // TODO: emit
    void startedFetchingChannel(const ChannelId &id);
    void countryUpdated(const CountryId &id);
    void channelUpdated(const ChannelId &id);
    void countryDetailsUpdated(const CountryId &id); // TODO: emit
    void channelDetailsUpdated(const ChannelId &id, const QString &image);
    void errorFetching(const Error &error);
    void errorFetchingCountry(const CountryId &id, const Error &error);
    void errorFetchingChannel(const ChannelId &id, const Error &error);
    void errorFetchingProgram(const ProgramId &id, const Error &error);
    void imageDownloadFinished(const QString &url);
};
