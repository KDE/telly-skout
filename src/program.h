/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QStringList>

#include "channel.h"
#include "country.h"

class Program : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QVector<Country *> countries READ countries CONSTANT)
    Q_PROPERTY(QDateTime start READ start CONSTANT)
    Q_PROPERTY(QDateTime stop READ stop CONSTANT)
    Q_PROPERTY(QString subtitle READ subtitle CONSTANT)
    Q_PROPERTY(QString baseUrl READ baseUrl CONSTANT);

public:
    Program(Channel *channel, int index);
    ~Program();

    QString id() const;
    QString title() const;
    QString description() const;
    QVector<Country *> countries() const;
    QDateTime start() const;
    QDateTime stop() const;
    QString subtitle() const;

    QString baseUrl() const;

private:
    Channel *m_channel;
    QString m_id;
    QString m_title;
    QString m_description;
    QVector<Country *> m_countries;
    QDateTime m_start;
    QDateTime m_stop;
    QString m_subtitle;
};

#endif // PROGRAM_H
