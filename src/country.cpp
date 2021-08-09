/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "country.h"

Country::Country(const QString &name, const QString &email, const QString &url, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_email(email)
    , m_url(url)
{
}

Country::~Country()
{
}

QString Country::name() const
{
    return m_name;
}

QString Country::email() const
{
    return m_email;
}

QString Country::url() const
{
    return m_url;
}
