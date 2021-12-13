#include "programfactory.h"

#include "database.h"
#include "fetcher.h"
#include "program.h"

#include <QDebug>
#include <QSqlQuery>

ProgramFactory::ProgramFactory()
    : QObject(nullptr)
{
    m_programs = Database::instance().programs();

    // TODO: possible to do this here and not in ProgramsModel?
    // issues: race condition -> data must be loaded when ProgramsModel::data() is called
    // connect(&Fetcher::instance(), &Fetcher::channelUpdated, this, [this](const QString &id) {
    //    load(id);
    // });
}

Program *ProgramFactory::create(const QString &channelId, int index) const
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

void ProgramFactory::load(const QString &channelId) const
{
    if (m_programs.contains(channelId)) {
        m_programs.remove(channelId);
    }
    m_programs[channelId] = Database::instance().programs(channelId);
}
