#pragma once

#include <QSortFilterProxyModel>

#include "types.h"

/**
 * @brief Filters and sorts ChannelsModel
 *
 */
class ChannelsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString groupName READ groupName WRITE setGroupName NOTIFY groupNameChanged)
    Q_PROPERTY(QString country READ country WRITE setCountry NOTIFY countryChanged)

public:
    explicit ChannelsProxyModel(QObject *parent = nullptr);
    ~ChannelsProxyModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QString groupName() const;
    void setGroupName(const QString &name);

    const QString &country() const;
    void setCountry(const QString &country);

Q_SIGNALS:
    void groupNameChanged();
    void countryChanged();

private:
    QString m_group_name; // TODO: favorite bool
    CountryId m_country;
};
