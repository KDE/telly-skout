// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "programdata.h"

#include <QDateTime>
#include <QString>

class Channel;

class Program : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ChannelId channelId READ channelId CONSTANT)
    Q_PROPERTY(ProgramId id READ id CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(bool descriptionFetched READ descriptionFetched CONSTANT)
    Q_PROPERTY(QDateTime start READ start CONSTANT)
    Q_PROPERTY(QDateTime stop READ stop CONSTANT)
    Q_PROPERTY(QString subtitle READ subtitle CONSTANT)
    Q_PROPERTY(QVector<QString> categories READ categories CONSTANT)

public:
    explicit Program(const ProgramData &data);
    ~Program() = default;

    const ChannelId &channelId() const;
    const ProgramId &id() const;
    QString url() const;
    QString title() const;
    QString description() const;
    bool descriptionFetched() const;
    QDateTime start() const;
    void setStart(const QDateTime &start);
    QDateTime stop() const;
    QString subtitle() const;
    QVector<QString> categories() const;

private:
    ProgramData m_data;
};
