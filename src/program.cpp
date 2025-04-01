// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "program.h"

#include <QDebug>

Program::Program(const ProgramData &data)
    : QObject(nullptr)
    , m_data(data)
{
}

const ChannelId &Program::channelId() const
{
    return m_data.m_channelId;
}

const ProgramId &Program::id() const
{
    return m_data.m_id;
}

QString Program::url() const
{
    return m_data.m_url;
}

QString Program::title() const
{
    return m_data.m_title;
}

QString Program::description() const
{
    return m_data.m_description;
}

bool Program::descriptionFetched() const
{
    return m_data.m_descriptionFetched;
}

QDateTime Program::start() const
{
    return m_data.m_startTime;
}

void Program::setStart(const QDateTime &start)
{
    m_data.m_startTime = start;
}

QDateTime Program::stop() const
{
    return m_data.m_stopTime;
}

void Program::setStop(const QDateTime &stop)
{
    m_data.m_stopTime = stop;
}

QString Program::subtitle() const
{
    return m_data.m_subtitle;
}

QVector<QString> Program::categories() const
{
    return m_data.m_categories;
}

void Program::setDescription(const QString &description)
{
    m_data.m_description = description;
    setDescriptionFetched(true);
    Q_EMIT descriptionChanged(m_data.m_description);
}

void Program::setDescriptionFetched(bool descriptionFetched)
{
    m_data.m_descriptionFetched = descriptionFetched;
    Q_EMIT descriptionFetchedChanged(m_data.m_descriptionFetched);
}

#include "moc_program.cpp"
