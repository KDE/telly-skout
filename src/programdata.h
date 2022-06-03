// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "types.h"

#include <QDateTime>
#include <QString>
#include <QVector>

struct ProgramData {
    ProgramId m_id;
    QString m_url;
    ChannelId m_channelId;
    QDateTime m_startTime;
    QDateTime m_stopTime;
    QString m_title;
    QString m_subtitle;
    QString m_description;
    bool m_descriptionFetched;
    QVector<QString> m_categories;
};
