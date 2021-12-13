#pragma once

#include <QObject>

#include "programdata.h"

#include <QMap>
#include <QVector>

class Program;

class ProgramFactory : public QObject
{
    Q_OBJECT

public:
    ProgramFactory();
    ~ProgramFactory() = default;

    size_t count(const QString &channelId) const;
    Program *create(const QString &channelId, int index) const;
    void load(const QString &channelId) const;

private:
    mutable QMap<QString, QVector<ProgramData>> m_programs; // list of programs per channel
};
