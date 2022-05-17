// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QString>

struct ChannelTag {
};
struct GroupTag {
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
using GroupId = QStringId<GroupTag>;
using ProgramId = QStringId<ProgramTag>;

class Error
{
public:
    Error()
        : m_id(0)
        , m_message("")
    {
    }

    Error(int id, const QString &message)
        : m_id(id)
        , m_message(message)
    {
    }

    void reset()
    {
        m_id = 0;
        m_message = "";
    }

    int m_id;
    QString m_message;
};
