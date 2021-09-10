# SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
# SPDX-License-Identifier: BSD-2-Clause

add_executable(telly-skout
    main.cpp
    channelsmodel.cpp
    channelstablemodel.cpp
    fetcher.cpp
    database.cpp
    program.cpp
    channel.cpp
    country.cpp
    countriesmodel.cpp
    channelsproxymodel.cpp
    resources.qrc
)

kconfig_add_kcfg_files(telly-skout TellySkoutSettings.kcfgc GENERATE_MOC)

target_include_directories(telly-skout PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(telly-skout PRIVATE Qt5::Core Qt5::Qml Qt5::Quick Qt5::QuickControls2 Qt5::Sql Qt5::Widgets KF5::CoreAddons KF5::ConfigGui KF5::I18n)

install(TARGETS telly-skout ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
