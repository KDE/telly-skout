// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "groupfactory.h"

#include "database.h"
#include "fetcher.h"
#include "group.h"

#include <QDebug>

GroupFactory::GroupFactory()
    : QObject(nullptr)
    , m_groups(Database::instance().groups())
{
}

size_t GroupFactory::count() const
{
    return m_groups.size();
}

Group *GroupFactory::create(int index) const
{
    // try to load if not avaible
    if (m_groups.size() <= index) {
        load();
    }
    // check if requested data exists
    if (m_groups.size() <= index) {
        return nullptr;
    }
    return new Group(m_groups.at(index));
}

void GroupFactory::load() const
{
    m_groups.clear();
    m_groups = Database::instance().groups();
}
