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
    Q_INVOKABLE void fetchChannels();
    Q_INVOKABLE void fetchCountry(const QString &url, const QString &countryId);
    Q_INVOKABLE void fetchChannel(const QString &channelId, const QString &name, const QString &country);
    Q_INVOKABLE void fetchProgram(const QString &channelId);
    Q_INVOKABLE QString image(const QString &url);
    void removeImage(const QString &url);
    Q_INVOKABLE void download(const QString &url);
    QNetworkReply *get(QNetworkRequest &request);

private:
    Fetcher();

    QString filePath(const QString &url);
    void processCountry(const QDomElement &country);
    void processChannel(const QDomElement &channel, const QString &url);
    void processProgram(const QDomNode &program, const QString &url);

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
