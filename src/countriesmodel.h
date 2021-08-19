/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QSqlTableModel>
#include <QUrl>

#include "channel.h"

class CountriesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit CountriesModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    Q_INVOKABLE void refreshAll();

private:
    void loadCountry(int index) const;

    mutable QVector<Country *> m_countries;
};
