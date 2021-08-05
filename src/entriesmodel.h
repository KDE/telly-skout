/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QObject>
#include <QString>

#include "channel.h"
#include "entry.h"

class EntriesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(Channel *channel READ channel CONSTANT)

public:
    explicit EntriesModel(Channel *channel);
    ~EntriesModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;

    Channel *channel() const;

private:
    void loadEntry(int index) const;

    Channel *m_channel;
    mutable QHash<int, Entry *> m_entries;
};
