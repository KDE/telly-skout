// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QSortFilterProxyModel>

#include <QDateTime>
#include <QQmlEngine>

class ProgramsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QDateTime start READ start WRITE setStart NOTIFY startChanged)
    Q_PROPERTY(QDateTime stop READ stop WRITE setStop NOTIFY stopChanged)

public:
    explicit ProgramsProxyModel(QObject *parent = nullptr);
    ~ProgramsProxyModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QDateTime start() const;
    void setStart(const QDateTime &start);

    QDateTime stop() const;
    void setStop(const QDateTime &stop);

Q_SIGNALS:
    void startChanged();
    void stopChanged();

private:
    QDateTime m_start;
    QDateTime m_stop;
};
