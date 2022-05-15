// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fetcherimpl.h"

#include <QtXml>

class XmltvFetcher : public FetcherImpl
{
    Q_OBJECT
public:
    XmltvFetcher();
    virtual ~XmltvFetcher() = default;

    void fetchCountries() override;
    void fetchCountry(const QString &url, const CountryId &countryId) override;
    void fetchProgram(const ChannelId &channelId) override;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override;

private:
    bool open(const QString &fileName);
    void fetchChannel(const ChannelId &channelId, const QString &name, const CountryId &countryId, const QString &icon);
    void processCountry(const QDomElement &country);
    void processProgram(const QDomNode &program);

    QDomDocument m_doc;
};
