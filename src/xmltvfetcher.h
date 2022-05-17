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

    void fetchGroups() override;
    void fetchGroup(const QString &url, const GroupId &groupId) override;
    void fetchProgram(const ChannelId &channelId) override;
    void fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url) override;

private:
    bool open(const QString &fileName);
    void fetchChannel(const ChannelId &channelId, const QString &name, const GroupId &groupId, const QString &icon);
    void processGroup(const QDomElement &group);
    void processProgram(const QDomNode &program);

    QDomDocument m_doc;
};
