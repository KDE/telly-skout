# SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
# SPDX-License-Identifier: BSD-2-Clause

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-telly-skout.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-telly-skout.h)

################# lib #################
add_library(telly-skout STATIC
    channel.cpp
    channelfactory.cpp
    channelsmodel.cpp
    channelsproxymodel.cpp
    dataprovider.h
    database.cpp
    fetcher.cpp
    fetcherimpl.h
    group.cpp
    groupfactory.cpp
    groupsmodel.cpp
    localdataprovider.cpp
    networkdataprovider.cpp
    networkfetcher.cpp
    program.cpp
    programfactory.cpp
    programsmodel.cpp
    programsproxymodel.cpp
    tvspielfilmfetcher.cpp
    xmltvfetcher.cpp
)

kconfig_add_kcfg_files(telly-skout TellySkoutSettings.kcfgc GENERATE_MOC)

target_include_directories(telly-skout PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(telly-skout PUBLIC Qt::Core Qt::Qml Qt::Quick Qt::QuickControls2 Qt::Sql KF5::CoreAddons KF5::ConfigGui KF5::I18n)

if(ANDROID)
    target_link_libraries(telly-skout PUBLIC KF5::Kirigami2)
else()
    target_link_libraries(telly-skout PUBLIC Qt::Widgets)
endif()

################# app #################
add_executable(telly-skout-app
    main.cpp
    resources.qrc
)

target_include_directories(telly-skout-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(telly-skout-app PRIVATE telly-skout)

set_target_properties(telly-skout-app PROPERTIES OUTPUT_NAME "telly-skout")

if(TARGET KF5::Crash)
    target_link_libraries(telly-skout-app PRIVATE KF5::Crash)
endif()

install(TARGETS telly-skout-app ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

if(ANDROID)
    kirigami_package_breeze_icons(ICONS
        favorite
        help-about-symbolic
        list-add
        settings-configure
        tv
        view-calendar-day
        view-refresh
        view-sort
    )
endif()
