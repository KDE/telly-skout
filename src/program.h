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
    Q_PROPERTY(QString content READ content CONSTANT)
    Q_PROPERTY(QVector<Country *> countries READ countries CONSTANT)
    Q_PROPERTY(QDateTime created READ created CONSTANT)
    Q_PROPERTY(QDateTime updated READ updated CONSTANT)
    Q_PROPERTY(QString link READ link CONSTANT)
    Q_PROPERTY(QString baseUrl READ baseUrl CONSTANT);

public:
    Program(Channel *channel, int index);
    ~Program();

    QString id() const;
    QString title() const;
    QString content() const;
    QVector<Country *> countries() const;
    QDateTime created() const;
    QDateTime updated() const;
    QString link() const;

    QString baseUrl() const;

    Q_INVOKABLE QString adjustedContent(int width, int fontSize);

private:
    Channel *m_channel;
    QString m_id;
    QString m_title;
    QString m_content;
    QVector<Country *> m_countries;
    QDateTime m_created;
    QDateTime m_updated;
    QString m_link;
};

#endif // PROGRAM_H
