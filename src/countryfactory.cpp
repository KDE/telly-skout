// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "countryfactory.h"

#include "country.h"
#include "database.h"
#include "fetcher.h"

#include <QDebug>

CountryFactory::CountryFactory()
    : QObject(nullptr)
    , m_countries(Database::instance().countries())
{
}

size_t CountryFactory::count() const
{
    return m_countries.size();
}

Country *CountryFactory::create(int index) const
{
    // try to load if not avaible
    if (m_countries.size() <= index) {
        load();
    }
    // check if requested data exists
    if (m_countries.size() <= index) {
        return nullptr;
    }
    return new Country(m_countries.at(index));
}

void CountryFactory::load() const
{
    m_countries.clear();
    m_countries = Database::instance().countries();
}
