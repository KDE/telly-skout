// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "channeldata.h"
#include "programfactory.h"
#include "types.h"

#include <QVector>

class Channel;

class ChannelFactory : public QObject
{
    Q_OBJECT

public:
    explicit ChannelFactory(bool onlyFavorites);
    ~ChannelFactory() = default;

    void setOnlyFavorites(bool onlyFavorites);
    size_t count() const;
    Channel *create(int index) const;
    void load() const;
    void update(const ChannelId &id);

private:
    mutable QVector<ChannelData> m_channels;
    bool m_onlyFavorites;
    mutable ProgramFactory m_programFactory;
};
