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
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE void refreshAll();

private:
    void load() const;
    void loadChannel(int index) const;
    qint64 timestampToday() const;
    qint64 timestamp(int row) const;
    int row(qint64 timestamp) const;

    const static int numRows = 24 * 60; // 1 row per minute for complete day
    mutable QVector<Channel *> m_channels;
    mutable QMap<int, std::array<Program *, numRows>> m_programs;
    mutable QMap<int, std::array<bool, numRows>> m_isFirst;
};
