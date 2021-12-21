#include "program.h"

#include "channel.h"
#include "country.h"
#include "database.h"

#include <QDebug>

Program::Program(const ProgramData &data)
    : QObject(nullptr)
    , m_data(data)
{
}

const QString &Program::channelId() const
{
    return m_data.m_channelId.value();
}

const QString &Program::id() const
{
    return m_data.m_id.value();
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

QString Program::subtitle() const
{
    return m_data.m_subtitle;
}

QString Program::category() const
{
    return m_data.m_category;
}
