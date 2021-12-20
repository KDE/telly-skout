/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "programdata.h"

#include <QObject>

class QDomElement;
class QDomNode;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QString;

class TvSpielfilmFetcher : public QObject
{
    Q_OBJECT
public:
    TvSpielfilmFetcher();

    void fetchFavorites();
    void fetchCountries();
    void fetchCountry(const QString &url, const QString &countryId);
    void fetchChannel(const QString &channelId, const QString &name, const QString &country);
    void fetchProgramDescription(const QString &channelId, const QString &programId, const QString &url);

private:
    void fetchProgram(const QString &channelId);
    void fetchProgram(const QString &channelId, const QString &url);
    void processChannel(const QString &infoTable, const QString &url, const QString &channelId);
    ProgramData processProgram(const QRegularExpressionMatch &programMatch, const QString &url, const QString &channelId, bool isLast);
    void processDescription(const QString &descriptionPage, const QString &url, const QString &programId);
    QNetworkReply *get(QNetworkRequest &request); // TODO: not in every class

    QNetworkAccessManager *m_manager;
};
