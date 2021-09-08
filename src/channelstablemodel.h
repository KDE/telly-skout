/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QAbstractTableModel>
#include <QHash>
#include <QMap>
#include <QSqlTableModel>
#include <QUrl>

#include <array>

class Channel;
class Program;

class ChannelsTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ChannelsTableModel(QObject *parent = nullptr);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    int rowCount() const;
    int columnCount(const QModelIndex &parent) const override;
    int columnCount() const;
    Q_INVOKABLE void setChannelAsFavorite(const QString &url);
    Q_INVOKABLE void refreshAll();

private:
    void loadChannel(int index) const;

    const static int numRows = 24 * 60; // 1 row per minute for complete day
    mutable QVector<Channel *> m_channels;
    mutable QMap<int, std::array<Program *, numRows>> m_programs;
};
