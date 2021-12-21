#include "countriesmodel.h"

#include "country.h"
#include "database.h"
#include "fetcher.h"
#include "types.h"

#include <QDebug>
#include <QSqlQuery>

CountriesModel::CountriesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&Database::instance(), &Database::countryAdded, this, [this]() {
        m_countryFactory.load();
        beginInsertRows(QModelIndex(), rowCount(QModelIndex()) - 1, rowCount(QModelIndex()) - 1);
        endInsertRows();
    });

    connect(&Fetcher::instance(), &Fetcher::countryDetailsUpdated, this, [this](const CountryId &id) {
        m_countryFactory.load();
        for (int i = 0; i < m_countries.length(); i++) {
            if (m_countries[i]->id() == id.value()) {
                Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                break;
            }
        }
    });

    connect(&Database::instance(), &Database::countryDetailsUpdated, this, [this](const CountryId &id) {
        m_countryFactory.load();
        for (int i = 0; i < m_countries.length(); i++) {
            if (m_countries[i]->id() == id.value()) {
                Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                break;
            }
        }
    });
}

QHash<int, QByteArray> CountriesModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[0] = "country";
    return roleNames;
}

int CountriesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_countryFactory.count();
}

QVariant CountriesModel::data(const QModelIndex &index, int role) const
{
    if (role != 0) {
        return QVariant();
    }
    if (m_countries.length() <= index.row()) {
        loadCountry(index.row());
    }
    return QVariant::fromValue(m_countries[index.row()]);
}

void CountriesModel::loadCountry(int index) const
{
    m_countries += m_countryFactory.create(index);
}
