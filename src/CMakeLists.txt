# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-2-Clause

add_executable(telly-skout
    main.cpp
    channel.cpp
    channelfactory.cpp
    channelsmodel.cpp
    channelsproxymodel.cpp
    country.cpp
    countryfactory.cpp
    countriesmodel.cpp
    database.cpp
    fetcher.cpp
    fetcherimpl.h
    networkfetcher.cpp
    program.cpp
    programfactory.cpp
    programsmodel.cpp
    programsproxymodel.cpp
    tvspielfilmfetcher.cpp
    xmltvfetcher.cpp
    xmltvsefetcher.cpp
    resources.qrc
)

kconfig_add_kcfg_files(telly-skout TellySkoutSettings.kcfgc GENERATE_MOC)

target_include_directories(telly-skout PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(telly-skout PRIVATE Qt5::Core Qt5::Qml Qt5::Quick Qt5::QuickControls2 Qt5::Sql KF5::CoreAddons KF5::ConfigGui KF5::I18n)

if(ANDROID)
    target_link_libraries(telly-skout PRIVATE KF5::Kirigami2)
else()
    target_link_libraries(telly-skout PRIVATE Qt5::Widgets)
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
