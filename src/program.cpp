/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "program.h"

#include "channel.h"
#include "country.h"
#include "database.h"

#include <QDebug>
#include <QSqlQuery>
#include <QUrl>

Program::Program(const ProgramData &data)
    : QObject(nullptr)
    , m_data(data)
{
}

QString Program::id() const
{
    return m_data.m_id;
}

QString Program::title() const
{
    return m_data.m_title;
}

QString Program::description() const
{
    return m_data.m_description;
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

QString Program::baseUrl() const
{
    return QUrl(m_data.m_subtitle).adjusted(QUrl::RemovePath).toString();
}
