/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVector>
#include <QtXml>

class Fetcher : public QObject
{
    Q_OBJECT
public:
    static Fetcher &instance()
    {
        static Fetcher _instance;
        return _instance;
    }
    Q_INVOKABLE void fetch(const QString &url);
    Q_INVOKABLE void fetchAll();
    Q_INVOKABLE QString image(const QString &url);
    void removeImage(const QString &url);
    Q_INVOKABLE void download(const QString &url);
    QNetworkReply *get(QNetworkRequest &request);

private:
    Fetcher();

    QString filePath(const QString &url);
    void processChannel(const QDomElement &channel, const QString &url);
    void processProgram(const QDomNode &program, const QString &url);
    void processAuthor(const QString &url, unsigned int id);
    void processEnclosure(const QString &feedUrl, unsigned int id);

    QNetworkAccessManager *manager;

Q_SIGNALS:
    void startedFetchingFeed(const QString &url);
    void feedUpdated(const QString &url);
    void feedDetailsUpdated(const QString &url,
                            const QString &name,
                            const QString &image,
                            const QString &link,
                            const QString &description,
                            const QDateTime &lastUpdated);
    void error(const QString &url, int errorId, const QString &errorString);
    void imageDownloadFinished(const QString &url);
};
