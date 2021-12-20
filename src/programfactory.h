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

private:
    mutable QMap<ChannelId, QVector<ProgramData>> m_programs;
};
