/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include "programdata.h"

#include <QDateTime>
#include <QString>

class Channel;
class Country;

class Program : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString channelId READ channelId CONSTANT)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QDateTime start READ start CONSTANT)
    Q_PROPERTY(QDateTime stop READ stop CONSTANT)
    Q_PROPERTY(QString subtitle READ subtitle CONSTANT)

public:
    Program(const ProgramData &data);
    ~Program() = default;

    QString channelId() const;
    QString id() const;
    QString url() const;
    QString title() const;
    QString description() const;
    QDateTime start() const;
    void setStart(const QDateTime &start);
    QDateTime stop() const;
    QString subtitle() const;

private:
    ProgramData m_data;
};
