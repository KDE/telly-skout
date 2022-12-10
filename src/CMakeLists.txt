# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-2-Clause

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-telly-skout.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-telly-skout.h)

add_executable(telly-skout
    main.cpp
    channel.cpp
    channelfactory.cpp
    channelsmodel.cpp
    channelsproxymodel.cpp
    database.cpp
    fetcher.cpp
    fetcherimpl.h
    group.cpp
    groupfactory.cpp
    groupsmodel.cpp
    networkfetcher.cpp
    program.cpp
    programfactory.cpp
    programsmodel.cpp
    programsproxymodel.cpp
    tvspielfilmfetcher.cpp
    xmltvfetcher.cpp
    resources.qrc
)

kconfig_add_kcfg_files(telly-skout TellySkoutSettings.kcfgc GENERATE_MOC)

target_include_directories(telly-skout PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(telly-skout PRIVATE Qt::Core Qt::Qml Qt::Quick Qt::QuickControls2 Qt::Sql KF5::CoreAddons KF5::ConfigGui KF5::I18n)

if(ANDROID)
    target_link_libraries(telly-skout PRIVATE KF5::Kirigami2)
else()
    target_link_libraries(telly-skout PRIVATE Qt::Widgets)
endif()

if(TARGET KF5::Crash)
    target_link_libraries(telly-skout PRIVATE KF5::Crash)
endif()

install(TARGETS telly-skout ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

if(ANDROID)
    kirigami_package_breeze_icons(ICONS
        favorite
        help-about-symbolic
        list-add
        rss
        settings-configure
        view-refresh
    )
endif()
