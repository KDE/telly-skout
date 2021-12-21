#pragma once

#include <QSortFilterProxyModel>

#include "types.h"

class ChannelsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool onlyFavorites READ onlyFavorites WRITE setOnlyFavorites NOTIFY onlyFavoritesChanged)
    Q_PROPERTY(QString country READ country WRITE setCountry NOTIFY countryChanged)

public:
    explicit ChannelsProxyModel(QObject *parent = nullptr);
    ~ChannelsProxyModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool onlyFavorites() const;
    void setOnlyFavorites(const bool &onlyFavorites);

    const QString &country() const;
    void setCountry(const QString &country);

Q_SIGNALS:
    void onlyFavoritesChanged();
    void countryChanged();

private:
    bool m_onlyFavorites;
    CountryId m_country;
};
