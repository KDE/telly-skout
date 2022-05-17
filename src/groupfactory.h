// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

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
