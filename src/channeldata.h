// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "types.h"

#include <QString>

struct ChannelData {
    ChannelId m_id;
    QString m_name;
    QString m_url;
    QString m_image;
};
