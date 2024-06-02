// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "groupfactory.h"

#include "database.h"
#include "group.h"

#include <QDebug>

GroupFactory::GroupFactory()
    : QObject(nullptr)
    , m_groups(Database::instance().groups())
{
}

size_t GroupFactory::count() const
{
    return static_cast<size_t>(m_groups.size());
}

Group *GroupFactory::create(int index) const
{
    // try to load if not avaible
    if (m_groups.size() <= index) {
        load();

        // check if requested data exists
        // load() changes m_groups
        // cppcheck-suppress identicalInnerCondition
        if (m_groups.size() <= index) {
            return nullptr;
        }
    }

    return new Group(m_groups.at(index));
}

void GroupFactory::load() const
{
    m_groups.clear();
    m_groups = Database::instance().groups();
}

#include "moc_groupfactory.cpp"
