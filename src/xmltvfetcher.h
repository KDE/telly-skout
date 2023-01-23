// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "fetcherimpl.h"

#include "localdataprovider.h"

#include <QtXml>

class XmltvFetcher : public FetcherImpl
{
    Q_OBJECT
public:
    XmltvFetcher();
    virtual ~XmltvFetcher() = default;

    void fetchGroups() override;
    void fetchGroup(const QString &url, const GroupId &groupId) override;
    void fetchProgram(const ChannelId &channelId) override;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override;
    QString image(const QString &url) override;
    QString imagePath(const QString &url) override;

private:
    void open(QByteArray data);
    void fetchChannel(const ChannelId &channelId, const QString &name, const GroupId &groupId, const QString &icon);
    void processGroup(const QDomElement &group);
    void processProgram(const QDomNode &program);

    QDomDocument m_doc;
    const LocalDataProvider m_provider;
};
