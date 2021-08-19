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

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(QVector<QString> countries READ countries WRITE setCountries NOTIFY countriesChanged)
    Q_PROPERTY(bool refreshing READ refreshing WRITE setRefreshing NOTIFY refreshingChanged)
    Q_PROPERTY(bool notify READ notify WRITE setNotify NOTIFY notifyChanged)
    Q_PROPERTY(int programCount READ programCount NOTIFY programCountChanged)
    Q_PROPERTY(int errorId READ errorId WRITE setErrorId NOTIFY errorIdChanged)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)
    Q_PROPERTY(ProgramsModel *programs MEMBER m_programs CONSTANT)

public:
    Channel(int index);

    ~Channel();

    QString id() const;
    QString url() const;
    QString name() const;
    QString image() const;
    bool favorite() const;
    QVector<QString> countries() const;
    bool notify() const;
    int programCount() const;
    int errorId() const;
    QString errorString() const;

    bool refreshing() const;

    void setName(const QString &name);
    void setImage(const QString &image);
    void setFavorite(bool favorite);
    void setCountries(const QVector<QString> &countries);
    void setNotify(bool notify);
    void setRefreshing(bool refreshing);
    void setErrorId(int errorId);
    void setErrorString(const QString &errorString);

    Q_INVOKABLE void refresh();
    void setAsFavorite();
    void remove();

Q_SIGNALS:
    void nameChanged(const QString &name);
    void imageChanged(const QString &image);
    void favoriteChanged(bool favorite);
    void countriesChanged(const QVector<QString> &countries);
    void deleteAfterCountChanged(int count);
    void deleteAfterTypeChanged(int type);
    void notifyChanged(bool notify);
    void programCountChanged();
    void errorIdChanged(int &errorId);
    void errorStringChanged(const QString &errorString);

    void refreshingChanged(bool refreshing);

private:
    QString m_id;
    QString m_url;
    QString m_name;
    QString m_image;
    bool m_favorite;
    QVector<QString> m_countries;
    bool m_notify;
    int m_errorId;
    QString m_errorString;
    ProgramsModel *m_programs;

    bool m_refreshing = false;
};

#endif // CHANNEL_H
