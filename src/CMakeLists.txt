# SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
# SPDX-License-Identifier: BSD-2-Clause

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-telly-skout.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-telly-skout.h)

################# lib #################
add_library(telly-skout STATIC)
ecm_add_qml_module(telly-skout URI org.kde.TellySkout GENERATE_PLUGIN_SOURCE
    SOURCES
        channel.cpp channel.h
        channelfactory.cpp channelfactory.h
        channelsmodel.cpp channelsmodel.h
        channelsproxymodel.cpp channelsproxymodel.h
        dataprovider.h
        database.cpp database.h
        fetcher.cpp fetcher.h
        fetcherimpl.h
        group.cpp group.h
        groupfactory.cpp groupfactory.h
        groupsmodel.cpp groupsmodel.h
        localdataprovider.cpp localdataprovider.h
        networkdataprovider.cpp networkdataprovider.h
        networkfetcher.cpp networkfetcher.h
        program.cpp program.h
        programfactory.cpp programfactory.h
        programsmodel.cpp programsmodel.h
        programsproxymodel.cpp programsproxymodel.h
        tvspielfilmfetcher.cpp tvspielfilmfetcher.h
        xmltvfetcher.cpp xmltvfetcher.h
)
target_link_libraries(telly-skoutplugin PUBLIC telly-skout)

kconfig_add_kcfg_files(telly-skout TellySkoutSettings.kcfgc GENERATE_MOC)
# WORKAROUND for parallel build (missing dependency)
# Without this, parallel builds (e.g. in CI) can fail with:
# cc1plus: error: /builds/utilities/telly-skout/_build/src/telly-skout_autogen/include: No such file or directory [-Werror=missing-include-dirs]
add_dependencies(telly-skout_resources_1 telly-skout_autogen)

target_link_libraries(telly-skout PUBLIC Qt::Core Qt::Qml Qt::Quick Qt::QuickControls2 Qt::Sql Qt::Xml KF${QT_MAJOR_VERSION}::CoreAddons KF${QT_MAJOR_VERSION}::ConfigGui KF${QT_MAJOR_VERSION}::I18n)

if(ANDROID)
    target_link_libraries(telly-skout PUBLIC KF${QT_MAJOR_VERSION}::Kirigami2)
else()
    target_link_libraries(telly-skout PUBLIC Qt::Widgets)
endif()

################# app #################
add_executable(telly-skout-app
    main.cpp
)

ecm_add_qml_module(telly-skout-app URI org.kde.TellySkout.app NO_RESOURCE_TARGET_PATH
    QML_FILES
        qml/ChannelListDelegate.qml
        qml/ChannelListPage.qml
        qml/FavoritesPage.qml
        qml/GroupListDelegate.qml
        qml/GroupListPage.qml
        qml/ProgramDelegate.qml
        qml/SettingsPage.qml
        qml/TellySkoutGlobalDrawer.qml
        qml/main.qml
)

target_include_directories(telly-skout-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(telly-skout-app PRIVATE telly-skoutplugin)

set_target_properties(telly-skout-app PROPERTIES RUNTIME_OUTPUT_NAME "telly-skout")

if(TARGET KF${QT_MAJOR_VERSION}::Crash)
    target_link_libraries(telly-skout-app PRIVATE KF${QT_MAJOR_VERSION}::Crash)
endif()

install(TARGETS telly-skout-app ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

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
