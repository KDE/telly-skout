// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "programdata.h"
#include "types.h"

#include <QMap>
#include <QVector>

class Program;

class ProgramFactory : public QObject
{
    Q_OBJECT

public:
    ProgramFactory();
    ~ProgramFactory() = default;

    size_t count(const ChannelId &channelId) const;
    Program *create(const ChannelId &channelId, int index) const;
    void load(const ChannelId &channelId) const;
    void load(const ChannelId &channelId, const ProgramId &programId) const;

private:
    mutable QMap<ChannelId, QVector<ProgramData>> m_programs;
};
