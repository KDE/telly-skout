#include "channelfactory.h"

#include "channel.h"
#include "database.h"
#include "fetcher.h"

#include <QDebug>
#include <QSqlQuery>

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
    if (onlyFavorites) {
        // try to load if not avaible
        if (m_favorites.size() <= index) {
            load(onlyFavorites);
        }
        // check if requested data exists
        if (m_favorites.size() <= index) {
            return nullptr;
        }
        return new Channel(m_favorites.at(index));
    } else {
        // try to load if not avaible
        if (m_channels.size() <= index) {
            load(onlyFavorites);
        }
        // check if requested data exists
        if (m_channels.size() <= index) {
            return nullptr;
        }
        return new Channel(m_channels.at(index));
    }
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
