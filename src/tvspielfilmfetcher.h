/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "programdata.h"
#include "types.h"

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
    void fetchCountry(const QString &url, const CountryId &countryId);
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url);

private:
    void fetchChannel(const ChannelId &channelId, const QString &name, const CountryId &country);
    void fetchProgram(const ChannelId &channelId);
    void fetchProgram(const ChannelId &channelId, const QString &url);
    void processChannel(const QString &infoTable, const QString &url, const ChannelId &channelId);
    ProgramData processProgram(const QRegularExpressionMatch &programMatch, const QString &url, const ChannelId &channelId, bool isLast);
    void processDescription(const QString &descriptionPage, const QString &url, const ProgramId &programId);
    QNetworkReply *get(QNetworkRequest &request); // TODO: not in every class

    QNetworkAccessManager *m_manager;
};
