// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "channeldata.h"
#include "programsmodel.h"

#include <QVector>

class Program;
class ProgramFactory;

class Channel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ChannelId id READ id CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(QVector<GroupId> groups READ groups WRITE setGroups NOTIFY groupsChanged)
    Q_PROPERTY(bool refreshing READ refreshing WRITE setRefreshing NOTIFY refreshingChanged)
    Q_PROPERTY(int errorId READ errorId NOTIFY errorIdChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(ProgramsModel *programsModel MEMBER m_programsModel CONSTANT)

public:
    Channel(const ChannelData &data, bool favorite, const QVector<GroupId> &groupIds, ProgramFactory &programFactory);

    ~Channel();

    ChannelId id() const;
    QString url() const;
    QString name() const;
    QString image() const;
    bool favorite() const;
    QVector<GroupId> groups() const;
    int programCount() const;
    int errorId() const;
    QString errorString() const;

    bool refreshing() const;

    void setName(const QString &name);
    void setImage(const QString &image);
    void setFavorite(bool favorite);
    void setGroups(const QVector<GroupId> &groups);
    void setRefreshing(bool refreshing);

Q_SIGNALS:
    void nameChanged(const QString &name);
    void imageChanged(const QString &image);
    void favoriteChanged(bool favorite);
    void groupsChanged(const QVector<GroupId> &groups);
    void deleteAfterCountChanged(int count);
    void deleteAfterTypeChanged(int type);
    void programChanged();
    void errorIdChanged(int errorId);
    void errorStringChanged(const QString &errorString);

    void refreshingChanged(bool refreshing);

private:
    void setError(const Error &error);

    ChannelData m_data;
    bool m_favorite;
    QVector<GroupId> m_groups;
    ProgramsModel *m_programsModel;
    Error m_error;

    bool m_refreshing = false;
};
