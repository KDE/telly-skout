/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "programdata.h"
#include "tvspielfilmfetcher.h"
#include "types.h"

#include <QObject>

class QDomElement;
class QDomNode;
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
    Q_INVOKABLE void fetchCountries();
    Q_INVOKABLE void fetchCountry(const QString &url, const QString &countryId); // TODO type safe IDs in qml
    void fetchCountry(const QString &url, const CountryId &countryId);
    void fetchChannel(const ChannelId &channelId, const QString &name, const CountryId &country);
    Q_INVOKABLE void fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url); // TODO type safe IDs in qml
    Q_INVOKABLE QString image(const QString &url);
    Q_INVOKABLE void download(const QString &url);

private:
    Fetcher();

    QString filePath(const QString &url);
    void removeImage(const QString &url);
    QNetworkReply *get(QNetworkRequest &request);

    QNetworkAccessManager *m_manager;
    TvSpielfilmFetcher m_tvSpielfilmFetcher;

    // TODO: rework
    friend class TvSpielfilmFetcher;
    void emitStartedFetchingFavorites();
    void emitFinishedFetchingFavorites();
    void emitStartedFetchingCountry(const CountryId &id); // TODO: emit
    void emitStartedFetchingChannel(const ChannelId &id);
    void emitCountryUpdated(const CountryId &id);
    void emitChannelUpdated(const ChannelId &id);
    void emitCountryDetailsUpdated(const CountryId &id); // TODO: emit
    void emitChannelDetailsUpdated(const ChannelId &id, const QString &image);
    void emitError(const QString &id, int errorId, const QString &errorString);
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
    void error(const QString &id, int errorId, const QString &errorString);
    void imageDownloadFinished(const QString &url);
};
