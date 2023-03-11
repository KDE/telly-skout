// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "fetcherimpl.h"

#include "localdataprovider.h"

#include <QDomDocument>
#include <QMap>

class XmltvFetcher : public FetcherImpl
{
    Q_OBJECT
public:
    XmltvFetcher();
    virtual ~XmltvFetcher() = default;

    void fetchGroups(std::function<void(const QVector<GroupData> &)> callback = nullptr, std::function<void(const Error &)> errorCallback = nullptr) override;
    void fetchGroup(const QString &url,
                    const GroupId &groupId,
                    std::function<void(const QList<ChannelData> &)> callback = nullptr,
                    std::function<void(const Error &)> errorCallback = nullptr) override;
    void fetchProgram(const ChannelId &channelId,
                      std::function<void(const QVector<ProgramData> &)> callback = nullptr,
                      std::function<void(const Error &)> errorCallback = nullptr) override;
    void fetchProgramDescription(const ChannelId &channelId,
                                 const ProgramId &programId,
                                 const QString &url,
                                 std::function<void(const QString &)> callback = nullptr,
                                 std::function<void(const Error &)> errorCallback = nullptr) override;
    QString image(const QString &url, std::function<void()> callback = nullptr, std::function<void(const Error &)> errorCallback = nullptr) override;
    QString imagePath(const QString &url) override;

private:
    void open(QByteArray data);
    void fetchChannel(const ChannelId &channelId, const QString &name, const QString &icon, QMap<ChannelId, ChannelData> &channels);
    ProgramData processProgram(const QDomNode &program);

    QDomDocument m_doc;
    const LocalDataProvider m_provider;
};
