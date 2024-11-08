# SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
# SPDX-License-Identifier: BSD-2-Clause

add_test(NAME telly-skout-self-test COMMAND telly-skout --self-test)
if (NOT BUILD_SHARED_LIBS)
    set_tests_properties(telly-skout-self-test PROPERTIES
        ENVIRONMENT "QT_QUICK_CONTROLS_STYLE=org.kde.breeze"
    )
endif()

ecm_add_test(databasetest.cpp LINK_LIBRARIES Qt::Test telly-skout)

ecm_add_test(fetchertest.cpp LINK_LIBRARIES Qt::Test telly-skout)

ecm_add_test(tvspielfilmfetchertest.cpp LINK_LIBRARIES Qt::Test Qt::Network telly-skout)

ecm_add_test(xmltvfetchertest.cpp LINK_LIBRARIES Qt::Test telly-skout)
target_include_directories(xmltvfetchertest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
