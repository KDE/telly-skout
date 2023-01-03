// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "groupdata.h"

#include <QVector>

class Group;

class GroupFactory : public QObject
{
    Q_OBJECT

public:
    GroupFactory();
    ~GroupFactory() = default;

    size_t count() const;
    Group *create(int index) const;
    void load() const;

private:
    mutable QVector<GroupData> m_groups;
};
