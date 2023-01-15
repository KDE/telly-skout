# SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_test(tvspielfilmfetchertest.cpp LINK_LIBRARIES Qt::Test Qt::Network telly-skout)

ecm_add_test(xmltvfetchertest.cpp LINK_LIBRARIES Qt::Test telly-skout)
target_include_directories(xmltvfetchertest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
