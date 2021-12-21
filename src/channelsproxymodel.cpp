#include "channelsproxymodel.h"

#include "channel.h"
#include "database.h"

ChannelsProxyModel::ChannelsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_onlyFavorites(false)
    , m_country("")
{
    connect(&Database::instance(), &Database::channelDetailsUpdated, [this]() {
        invalidateFilter();
    });
}

ChannelsProxyModel::~ChannelsProxyModel()
{
}

bool ChannelsProxyModel::onlyFavorites() const
{
    return m_onlyFavorites;
}

void ChannelsProxyModel::setOnlyFavorites(const bool &onlyFavorites)
{
    if (m_onlyFavorites != onlyFavorites) {
        m_onlyFavorites = onlyFavorites;
        invalidateFilter();
        Q_EMIT onlyFavoritesChanged();
    }
}

const QString &ChannelsProxyModel::country() const
{
    return m_country.value();
}

void ChannelsProxyModel::setCountry(const QString &country)
{
    if (m_country.value() != country) {
        m_country = CountryId(country);
        invalidateFilter();
        Q_EMIT countryChanged();
    }
}

bool ChannelsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto idx = sourceModel()->index(source_row, 0, source_parent);

    if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)) {
        return false;
    }

    // no filter
    if (!m_onlyFavorites && m_country.value().isEmpty()) {
        return true;
    }

    // at least one filter
    auto channel = idx.data(0).value<Channel *>();

    const bool onlyFavoritesMatches = !m_onlyFavorites || channel->favorite();
    const bool countryMatches = m_country.value().isEmpty() || channel->countries().contains(m_country.value());

    return onlyFavoritesMatches && countryMatches;
}
