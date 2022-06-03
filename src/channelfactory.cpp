// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "channelfactory.h"

#include "channel.h"
#include "database.h"
#include "fetcher.h"
#include "groupdata.h"

#include <QDebug>

#include <algorithm>

ChannelFactory::ChannelFactory(bool onlyFavorites)
    : QObject(nullptr)
    , m_onlyFavorites(onlyFavorites)
{
    load();
}

void ChannelFactory::setOnlyFavorites(bool onlyFavorites)
{
    if (m_onlyFavorites != onlyFavorites) {
        m_onlyFavorites = onlyFavorites;
        load();
    }
}

size_t ChannelFactory::count() const
{
    return m_channels.size();
}

Channel *ChannelFactory::create(int index) const
{
    // try to load if not avaible
    if (m_channels.size() <= index) {
        load();
    }
    // check if requested data exists
    if (m_channels.size() <= index) {
        return nullptr;
    }
    const ChannelData &data = m_channels.at(index);

    // check if channel is favorite
    // if onlyFavorites == true, it must be a favorite
    // but onlyFavorites == false does not mean that it cannot be favorite
    bool favorite = m_onlyFavorites;
    if (!m_onlyFavorites) {
        favorite = Database::instance().isFavorite(data.m_id);
    }

    const QVector<GroupData> groups = Database::instance().groups(data.m_id);
    QVector<QString> groupIds(groups.size());
    std::transform(groups.begin(), groups.end(), groupIds.begin(), [](const GroupData &data) {
        return data.m_id.value();
    });

    return new Channel(data, favorite, groupIds, m_programFactory);
}

void ChannelFactory::load() const
{
    m_channels.clear();
    m_channels = Database::instance().channels(m_onlyFavorites);
}

void ChannelFactory::update(const ChannelId &id)
{
    if (m_onlyFavorites) {
        // remove if no favorite anymore
        if (!Database::instance().isFavorite(id)) {
            QVector<ChannelData>::iterator it = std::find_if(m_channels.begin(), m_channels.end(), [id](const ChannelData &data) {
                return data.m_id == id;
            });
            if (it != m_channels.end()) {
                m_channels.erase(it);
            }
        } else {
            // reload favorites if favorite added or maybe favorite order changed
            load();
        }
    } else {
        QVector<ChannelData>::iterator it = std::find_if(m_channels.begin(), m_channels.end(), [id](const ChannelData &data) {
            return data.m_id == id;
        });
        if (it != m_channels.end()) {
            *it = Database::instance().channel(id);
        } else {
            m_channels.append(Database::instance().channel(id));
        }
    }
}
