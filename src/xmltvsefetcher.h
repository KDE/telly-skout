/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include "types.h"

class QDomElement;
class QDomNode;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QString;

class XmlTvSeFetcher : public QObject
{
    Q_OBJECT
public:
    XmlTvSeFetcher();
    void fetchFavorites();
    void fetchCountries();
    void fetchCountry(const QString &url, const CountryId &countryId);
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url);

private:
    void fetchChannel(const ChannelId &channelId, const QString &name, const CountryId &countryId);
    void fetchProgram(const ChannelId &channelId);
    void processCountry(const QDomElement &country);
    void processChannel(const QDomElement &channel, const QString &url);
    void processProgram(const QDomNode &program, const QString &url);
    QNetworkReply *get(QNetworkRequest &request);

    QNetworkAccessManager *manager;
};
