#include "channelfactory.h"

#include "channel.h"
#include "countrydata.h"
#include "database.h"
#include "fetcher.h"

#include <QDebug>

#include <algorithm>

ChannelFactory::ChannelFactory()
    : QObject(nullptr)
{
    m_channels = Database::instance().channels(false);
    m_favorites = Database::instance().channels(true);
}

size_t ChannelFactory::count(bool onlyFavorites) const
{
    if (onlyFavorites) {
        return m_favorites.size();
    } else {
        return m_channels.size();
    }
}

Channel *ChannelFactory::create(bool onlyFavorites, int index) const
{
    const ChannelData *data = nullptr;
    if (onlyFavorites) {
        // try to load if not avaible
        if (m_favorites.size() <= index) {
            load(onlyFavorites);
        }
        // check if requested data exists
        if (m_favorites.size() <= index) {
            return nullptr;
        }
        data = &(m_favorites.at(index));
    } else {
        // try to load if not avaible
        if (m_channels.size() <= index) {
            load(onlyFavorites);
        }
        // check if requested data exists
        if (m_channels.size() <= index) {
            return nullptr;
        }
        data = &(m_channels.at(index));
    }

    // to be safe, cannot happen by design
    if (!data) {
        return nullptr;
    }

    // check if channel is favorite
    // (!onlyFavorites does not mean that it cannot be favorite)
    const bool favorite = Database::instance().isFavorite(data->m_id);

    const QVector<CountryData> countries = Database::instance().countries(data->m_id);
    QVector<QString> countryIds(countries.size());
    std::transform(countries.begin(), countries.end(), countryIds.begin(), [](const CountryData &data) {
        return data.m_id.value();
    });

    return new Channel(*data, favorite, countryIds);
}

void ChannelFactory::load(bool onlyFavorites) const
{
    if (onlyFavorites) {
        m_favorites.clear();
        m_favorites = Database::instance().channels(onlyFavorites);
    } else {
        m_channels.clear();
        m_channels = Database::instance().channels(onlyFavorites);
    }
}

void ChannelFactory::update(const ChannelId &id)
{
    const ChannelData newData = Database::instance().channel(id);
    QVector<ChannelData>::iterator it = std::find_if(m_favorites.begin(), m_favorites.end(), [id](const ChannelData &data) {
        return data.m_id == id;
    });
    // remove if no favorite anymore
    if (it != m_favorites.end() && !Database::instance().isFavorite(id)) {
        m_favorites.erase(it);
    } else {
        // reload favorites if favorite added or maybe favorite order changed
        load(true);
    }

    it = std::find_if(m_channels.begin(), m_channels.end(), [id](const ChannelData &data) {
        return data.m_id == id;
    });
    if (it != m_channels.end()) {
        *it = newData;
    }
}
