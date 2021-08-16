/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "program.h"

#include <QRegularExpression>
#include <QSqlQuery>
#include <QUrl>

#include "database.h"

Program::Program(Channel *channel, int index)
    : QObject(nullptr)
    , m_channel(channel)
{
    QSqlQuery programQuery;
    programQuery.prepare(QStringLiteral("SELECT * FROM Programs WHERE channel=:channel ORDER BY start LIMIT 1 OFFSET :index;"));
    programQuery.bindValue(QStringLiteral(":channel"), m_channel->url());
    programQuery.bindValue(QStringLiteral(":index"), index);
    Database::instance().execute(programQuery);
    if (!programQuery.next()) {
        qWarning() << "No element with index" << index << "found in channel" << m_channel->url();
    }

    QSqlQuery countryQuery;
    countryQuery.prepare(QStringLiteral("SELECT * FROM Countries WHERE id=:id"));
    countryQuery.bindValue(QStringLiteral(":id"), programQuery.value(QStringLiteral("id")).toString());
    Database::instance().execute(countryQuery);

    while (countryQuery.next()) {
        m_countries += new Country(countryQuery.value(QStringLiteral("name")).toString(), countryQuery.value(QStringLiteral("url")).toString(), nullptr);
    }

    m_start.setSecsSinceEpoch(programQuery.value(QStringLiteral("start")).toInt());
    m_stop.setSecsSinceEpoch(programQuery.value(QStringLiteral("stop")).toInt());

    m_id = programQuery.value(QStringLiteral("id")).toString();
    m_title = programQuery.value(QStringLiteral("title")).toString();
    m_description = programQuery.value(QStringLiteral("description")).toString();
    m_subtitle = programQuery.value(QStringLiteral("subtitle")).toString();
}

Program::~Program()
{
    qDeleteAll(m_countries);
}

QString Program::id() const
{
    return m_id;
}

QString Program::title() const
{
    return m_title;
}

QString Program::description() const
{
    return m_description;
}

QVector<Country *> Program::countries() const
{
    return m_countries;
}

QDateTime Program::start() const
{
    return m_start;
}

QDateTime Program::stop() const
{
    return m_stop;
}

QString Program::subtitle() const
{
    return m_subtitle;
}

QString Program::baseUrl() const
{
    return QUrl(m_subtitle).adjusted(QUrl::RemovePath).toString();
}
