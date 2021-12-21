#pragma once

#include <QObject>

#include "channeldata.h"

#include <QVector>

class Program;
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
    Q_PROPERTY(int errorId READ errorId NOTIFY errorIdChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(ProgramsModel *programsModel MEMBER m_programsModel CONSTANT)

public:
    Channel(const ChannelData &data, bool favorite, const QVector<QString> &countryIds);

    ~Channel();

    QString id() const;
    QString url() const;
    QString name() const;
    QString image() const;
    bool favorite() const;
    QVector<QString> countries() const;
    int programCount() const;
    int errorId() const;
    QString errorString() const;

    bool refreshing() const;

    void setName(const QString &name);
    void setImage(const QString &image);
    void setFavorite(bool favorite);
    void setCountries(const QVector<QString> &countries);
    void setRefreshing(bool refreshing);

Q_SIGNALS:
    void nameChanged(const QString &name);
    void imageChanged(const QString &image);
    void favoriteChanged(bool favorite);
    void countriesChanged(const QVector<QString> &countries);
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
    QVector<QString> m_countries;
    ProgramsModel *m_programsModel;
    Error m_error;

    bool m_refreshing = false;
};
