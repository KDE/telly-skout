/*
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include "types.h"

class ChannelsModel;

class Country : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool refreshing READ refreshing WRITE setRefreshing NOTIFY refreshingChanged)
    Q_PROPERTY(int channelCount READ channelCount NOTIFY channelCountChanged)
    Q_PROPERTY(int errorId READ errorId WRITE setErrorId NOTIFY errorIdChanged)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)
    Q_PROPERTY(ChannelsModel *channels MEMBER m_channels CONSTANT)

public:
    Country(int index);

    ~Country();

    QString id() const;
    QString url() const;
    QString name() const;
    int channelCount() const;
    int errorId() const;
    QString errorString() const;

    bool refreshing() const;

    void setName(const QString &name);
    void setRefreshing(bool refreshing);
    void setErrorId(int errorId);
    void setErrorString(const QString &errorString);

    Q_INVOKABLE void refresh();
    void setAsFavorite();
    void remove();

Q_SIGNALS:
    void nameChanged(const QString &name);
    void countriesChanged(const QVector<Country *> &countries);
    void deleteAfterCountChanged(int count);
    void deleteAfterTypeChanged(int type);
    void channelCountChanged();
    void errorIdChanged(int &errorId);
    void errorStringChanged(const QString &errorString);

    void refreshingChanged(bool refreshing);

private:
    CountryId m_id;
    QString m_url;
    QString m_name;
    int m_errorId;
    QString m_errorString;
    ChannelsModel *m_channels;

    bool m_refreshing = false;
};
