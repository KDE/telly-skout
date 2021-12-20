#pragma once

#include "types.h"

#include <QDateTime>
#include <QString>

struct ProgramData {
    ProgramId m_id;
    QString m_url;
    ChannelId m_channelId;
    QDateTime m_startTime;
    QDateTime m_stopTime;
    QString m_title;
    QString m_subtitle;
    QString m_description;
    QString m_category;
};
