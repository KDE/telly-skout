// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "modelfactory.h"

#include "channelsmodel.h"
#include "channelsproxymodel.h"
#include "database.h"
#include "groupsmodel.h"
#include "programsmodel.h"
#include "programsproxymodel.h"

#include <QAbstractItemModel>
#include <QDateTime>
#include <QDebug>
#include <QString>

ModelFactory::ModelFactory()
    : QObject(nullptr)
{
}

GroupsModel *ModelFactory::createGroupsModel() const
{
    return new GroupsModel;
}

ChannelsModel *ModelFactory::createChannelsModel(bool onlyFavorites) const
{
    return new ChannelsModel(onlyFavorites);
}

ChannelsProxyModel *ModelFactory::createChannelsProxyModel(ChannelsModel *sourceModel, bool onlyFavorites, const QString &group) const
{
    return new ChannelsProxyModel(sourceModel, onlyFavorites, group);
}

ProgramsProxyModel *ModelFactory::createProgramsProxyModel(ProgramsModel *sourceModel, const QDateTime &start, const QDateTime &stop) const
{
    return new ProgramsProxyModel(sourceModel, start, stop);
}
