#pragma once

#include <QObject>

#include "countrydata.h"

#include <QVector>

class Country;

class CountryFactory : public QObject
{
    Q_OBJECT

public:
    CountryFactory();
    ~CountryFactory() = default;

    size_t count() const;
    Country *create(int index) const;
    void load() const;

private:
    mutable QVector<CountryData> m_countries;
};
