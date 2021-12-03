#pragma once

#include <QDateTime>
#include <QString>

struct ProgramData {
    QString m_id;
    QString m_url;
    QString m_channelId;
    QDateTime m_startTime;
    QDateTime m_stopTime;
    QString m_title;
    QString m_subtitle;
    QString m_description;
    QString m_category;
};
