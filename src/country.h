/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef COUNTRY_H
#define COUNTRY_H

#include <QObject>

class Country : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)

public:
    Country(const QString &name, const QString &url, QObject *parent = nullptr);
    ~Country();

    QString name() const;
    QString url() const;

private:
    QString m_name;
    QString m_url;
};

#endif // COUNTRY_H
