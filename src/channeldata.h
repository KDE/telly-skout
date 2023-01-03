// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "types.h"

#include <QString>

struct ChannelData {
    ChannelId m_id;
    QString m_name;
    QString m_url;
    QString m_image;
};
