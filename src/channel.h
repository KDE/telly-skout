/*
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#include <QDateTime>
#include <QObject>

#include "country.h"

class ProgramsModel;

class Channel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(QString link READ link WRITE setLink NOTIFY linkChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(QVector<Country *> countries READ countries WRITE setCountries NOTIFY countriesChanged)
    Q_PROPERTY(bool refreshing READ refreshing WRITE setRefreshing NOTIFY refreshingChanged)
    Q_PROPERTY(int deleteAfterCount READ deleteAfterCount WRITE setDeleteAfterCount NOTIFY deleteAfterCountChanged)
    Q_PROPERTY(int deleteAfterType READ deleteAfterType WRITE setDeleteAfterType NOTIFY deleteAfterTypeChanged)
    Q_PROPERTY(QDateTime subscribed READ subscribed CONSTANT)
    Q_PROPERTY(QDateTime lastUpdated READ lastUpdated WRITE setLastUpdated NOTIFY lastUpdatedChanged)
    Q_PROPERTY(bool notify READ notify WRITE setNotify NOTIFY notifyChanged)
    Q_PROPERTY(int programCount READ programCount NOTIFY programCountChanged)
    Q_PROPERTY(int unreadProgramCount READ unreadProgramCount NOTIFY unreadProgramCountChanged)
    Q_PROPERTY(int errorId READ errorId WRITE setErrorId NOTIFY errorIdChanged)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)
    Q_PROPERTY(ProgramsModel *programs MEMBER m_programs CONSTANT)

public:
    Channel(int index);

    ~Channel();

    QString url() const;
    QString name() const;
    QString displayName() const;
    QString image() const;
    QString link() const;
    QString description() const;
    bool favorite() const;
    QVector<Country *> countries() const;
    int deleteAfterCount() const;
    int deleteAfterType() const;
    QDateTime subscribed() const;
    QDateTime lastUpdated() const;
    bool notify() const;
    int programCount() const;
    int unreadProgramCount() const;
    bool read() const;
    int errorId() const;
    QString errorString() const;

    bool refreshing() const;

    void setName(const QString &name);
    void setDisplayName(const QString &displayName);
    void setImage(const QString &image);
    void setLink(const QString &link);
    void setDescription(const QString &description);
    void setFavorite(bool favorite);
    void setCountries(const QVector<Country *> &countries);
    void setDeleteAfterCount(int count);
    void setDeleteAfterType(int type);
    void setLastUpdated(const QDateTime &lastUpdated);
    void setNotify(bool notify);
    void setRefreshing(bool refreshing);
    void setErrorId(int errorId);
    void setErrorString(const QString &errorString);

    Q_INVOKABLE void refresh();
    void setAsFavorite();
    void remove();

Q_SIGNALS:
    void nameChanged(const QString &name);
    void displayNameChanged(const QString &displayName);
    void imageChanged(const QString &image);
    void linkChanged(const QString &link);
    void descriptionChanged(const QString &description);
    void favoriteChanged(bool favorite);
    void countriesChanged(const QVector<Country *> &countries);
    void deleteAfterCountChanged(int count);
    void deleteAfterTypeChanged(int type);
    void lastUpdatedChanged(const QDateTime &lastUpdated);
    void notifyChanged(bool notify);
    void programCountChanged();
    void unreadProgramCountChanged();
    void errorIdChanged(int &errorId);
    void errorStringChanged(const QString &errorString);

    void refreshingChanged(bool refreshing);

private:
    QString m_url;
    QString m_name;
    QString m_display_name;
    QString m_image;
    QString m_link;
    QString m_description;
    bool m_favorite;
    QVector<Country *> m_countries;
    int m_deleteAfterCount;
    int m_deleteAfterType;
    QDateTime m_subscribed;
    QDateTime m_lastUpdated;
    bool m_notify;
    int m_errorId;
    QString m_errorString;
    ProgramsModel *m_programs;

    bool m_refreshing = false;
};

#endif // CHANNEL_H
