// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include "programdata.h"

#include <QDateTime>
#include <QString>

class Channel;

class Program : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString channelId READ channelId CONSTANT)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description)
    Q_PROPERTY(bool descriptionFetched READ descriptionFetched)
    Q_PROPERTY(QDateTime start READ start CONSTANT)
    Q_PROPERTY(QDateTime stop READ stop CONSTANT)
    Q_PROPERTY(QString subtitle READ subtitle CONSTANT)
    Q_PROPERTY(QVector<QString> categories READ categories CONSTANT)

public:
    explicit Program(const ProgramData &data);
    ~Program() = default;

    const QString &channelId() const;
    const QString &id() const;
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
