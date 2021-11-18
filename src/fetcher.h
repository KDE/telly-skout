/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

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
    Q_INVOKABLE void fetchCountry(const QString &url, const QString &countryId);
    Q_INVOKABLE void fetchChannel(const QString &channelId, const QString &name, const QString &country);
    Q_INVOKABLE void fetchProgram(const QString &channelId);
    Q_INVOKABLE void fetchDescription(const QString &channelId, const QString &programId, const QString &descriptionUrl, bool isLast);
    Q_INVOKABLE QString image(const QString &url);
    void removeImage(const QString &url);
    Q_INVOKABLE void download(const QString &url);
    QNetworkReply *get(QNetworkRequest &request);

private:
    Fetcher();

    QString filePath(const QString &url);
    void fetchProgram(const QString &channelId, const QString &url);
    void processChannel(const QString &infoTable, const QString &url, const QString &channelId);
    void processProgram(const QString &programRow, const QString &url, const QString &channelId, bool isLast);
    void processDescription(const QString &descriptionPage, const QString &url, const QString &programId);

    QNetworkAccessManager *manager;

Q_SIGNALS:
    void startedFetchingFavorites();
    void finishedFetchingFavorites();
    void startedFetchingCountry(const QString &id); // TODO: emit
    void startedFetchingChannel(const QString &id);
    void countryUpdated(const QString &id);
    void channelUpdated(const QString &id);
    void countryDetailsUpdated(const QString &id); // TODO: emit
    void channelDetailsUpdated(const QString &id, const QString &image);
    void error(const QString &id, int errorId, const QString &errorString);
    void imageDownloadFinished(const QString &url);
};
