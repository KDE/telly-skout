/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QDebug>
#include <QModelIndex>
#include <QSqlQuery>
#include <QUrl>
#include <QVariant>

#include "countriesmodel.h"
#include "database.h"
#include "fetcher.h"

CountriesModel::CountriesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&Database::instance(), &Database::countryAdded, this, [this]() {
        beginInsertRows(QModelIndex(), rowCount(QModelIndex()) - 1, rowCount(QModelIndex()) - 1);
        endInsertRows();
    });

    connect(&Fetcher::instance(), &Fetcher::countryDetailsUpdated, this, [this](const QString &id) {
        for (int i = 0; i < m_countries.length(); i++) {
            if (m_countries[i]->id() == id) {
                Q_EMIT dataChanged(createIndex(i, 0), createIndex(i, 0));
                break;
            }
        }
    });

    connect(&Database::instance(), &Database::countryDetailsUpdated, [this](const QString &id) {
        for (int i = 0; i < m_countries.length(); i++) {
            if (m_countries[i]->id() == id) {
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
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT() FROM Countries;"));
    Database::instance().execute(query);
    if (!query.next()) {
        qWarning() << "Failed to query country count";
    }
    return query.value(0).toInt();
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
    m_countries += new Country(index);
}

void CountriesModel::refreshAll()
{
    for (auto &country : m_countries) {
        country->refresh();
    }
}
