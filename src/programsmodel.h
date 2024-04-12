// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QAbstractListModel>

#include <QHash>
#include <QObject>
#include <QQmlEngine>

class Channel;
class Program;
class ProgramFactory;

class ProgramsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Channel *channel READ channel CONSTANT)

public:
    explicit ProgramsModel(Channel *channel, ProgramFactory &programFactory);
    ~ProgramsModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;

    Channel *channel() const;

private:
    void loadProgram(int index) const;

    Channel *m_channel;
    mutable QHash<int, Program *> m_programs;
    ProgramFactory &m_programFactory;
};
