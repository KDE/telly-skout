// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>

#include "countryfactory.h"

class Country;

class CountriesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit CountriesModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;

private:
    void loadCountry(int index) const;

    mutable QVector<Country *> m_countries;
    CountryFactory m_countryFactory;
};
