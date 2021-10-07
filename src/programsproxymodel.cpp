/*
 * SPDX-FileCopyrightText: 2021 Dimitris Kardarakos <dimkard@posteo.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "programsproxymodel.h"

#include "program.h"

ProgramsProxyModel::ProgramsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_start{}
    , m_stop{}
{
}

ProgramsProxyModel::~ProgramsProxyModel()
{
}

QDateTime ProgramsProxyModel::start() const
{
    return m_start;
}

void ProgramsProxyModel::setStart(const QDateTime &start)
{
    if (m_start != start) {
        m_start = start;
        invalidateFilter();
        Q_EMIT startChanged();
    }
}

QDateTime ProgramsProxyModel::stop() const
{
    return m_stop;
}

void ProgramsProxyModel::setStop(const QDateTime &stop)
{
    if (m_stop != stop) {
        m_stop = stop;
        invalidateFilter();
        Q_EMIT stopChanged();
    }
}

bool ProgramsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto idx = sourceModel()->index(source_row, 0, source_parent);

    if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)) {
        return false;
    }

    // at least one filter
    auto program = idx.data(0).value<Program *>();

    const bool startOk = program->stop() > m_start;
    const bool stopOk = program->start() <= m_stop;

    return startOk && stopOk;
}
