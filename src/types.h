#pragma once

#include <QString>

struct ChannelTag {
};
struct CountryTag {
};
struct ProgramTag {
};

template<class Tag>
struct QStringId {
    explicit QStringId(const QString &id)
        : m_id(id)
    {
    }
    QStringId()
        : m_id()
    {
    }

    const QString &value() const
    {
        return m_id;
    }

private:
    QString m_id;

    friend bool operator==(const QStringId &l, const QStringId &r)
    {
        return l.m_id == r.m_id;
    }

    friend bool operator!=(const QStringId &l, const QStringId &r)
    {
        return !(l == r);
    }

    friend bool operator<(const QStringId &l, const QStringId &r)
    {
        return l.m_id < r.m_id;
    }
};

using ChannelId = QStringId<ChannelTag>;
using CountryId = QStringId<CountryTag>;
using ProgramId = QStringId<ProgramTag>;
