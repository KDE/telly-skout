// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include "countrydata.h"
#include "types.h"

class ChannelsModel;

class Country : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(bool refreshing READ refreshing WRITE setRefreshing NOTIFY refreshingChanged)
    Q_PROPERTY(int errorId READ errorId NOTIFY errorIdChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(ChannelsModel *channels MEMBER m_channels CONSTANT)

public:
    Country(CountryData data);

    ~Country();

    QString id() const;
    QString name() const;
    QString url() const;
    int errorId() const;
    QString errorString() const;

    bool refreshing() const;

    void setName(const QString &name);
    void setRefreshing(bool refreshing);

Q_SIGNALS:
    void nameChanged(const QString &name);
    void countriesChanged(const QVector<Country *> &countries);
    void deleteAfterCountChanged(int count);
    void deleteAfterTypeChanged(int type);
    void errorIdChanged(int errorId);
    void errorStringChanged(const QString &errorString);

    void refreshingChanged(bool refreshing);

private:
    void setError(const Error &error);

    CountryData m_data;
    ChannelsModel *m_channels;
    Error m_error;

    bool m_refreshing = false;
};
