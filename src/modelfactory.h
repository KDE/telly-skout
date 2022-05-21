// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

class ChannelsModel;
class ChannelsProxyModel;
class GroupsModel;
class ProgramsModel;
class ProgramsProxyModel;

class QDateTime;
class QString;

class ModelFactory : public QObject
{
    Q_OBJECT

public:
    ModelFactory();
    ~ModelFactory() = default;

    Q_INVOKABLE GroupsModel *createGroupsModel() const;
    Q_INVOKABLE ChannelsModel *createChannelsModel(bool onlyFavorites) const;
    Q_INVOKABLE ChannelsProxyModel *createChannelsProxyModel(ChannelsModel *sourceModel, bool onlyFavorites, const QString &group) const;
    Q_INVOKABLE ProgramsProxyModel *createProgramsProxyModel(ProgramsModel *sourceModel, const QDateTime &start, const QDateTime &stop) const;
};
