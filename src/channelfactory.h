// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

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
    ChannelFactory(bool onlyFavorites);
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
