// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "programfactory.h"

#include "database.h"
#include "fetcher.h"
#include "program.h"

#include <QDebug>

ProgramFactory::ProgramFactory()
    : QObject(nullptr)
{
    m_programs = Database::instance().programs();
}

size_t ProgramFactory::count(const ChannelId &channelId) const
{
    // try to load if not avaible
    if (!m_programs.contains(channelId)) {
        load(channelId);
    }
    // check if requested data exists
    if (!m_programs.contains(channelId)) {
        return 0;
    }
    return m_programs[channelId].size();
}

Program *ProgramFactory::create(const ChannelId &channelId, int index) const
{
    // try to load if not avaible
    if (!m_programs.contains(channelId)) {
        load(channelId);
    }
    // check if requested data exists
    if (!m_programs.contains(channelId) || m_programs[channelId].size() <= index) {
        return nullptr;
    }
    return new Program(m_programs[channelId].at(index));
}

void ProgramFactory::load(const ChannelId &channelId) const
{
    if (m_programs.contains(channelId)) {
        m_programs.remove(channelId);
    }
    m_programs[channelId] = Database::instance().programs(channelId);
}
