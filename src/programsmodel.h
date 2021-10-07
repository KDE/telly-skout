/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QAbstractListModel>

#include <QHash>
#include <QObject>

class Channel;
class Program;

class ProgramsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(Channel *channel READ channel CONSTANT)

public:
    explicit ProgramsModel(Channel *channel);
    ~ProgramsModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;

    Channel *channel() const;

private:
    void loadProgram(int index) const;

    Channel *m_channel;
    mutable QHash<int, Program *> m_programs;
};
