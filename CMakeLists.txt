# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

project(telly-skout)
set(PROJECT_VERSION "1.0")

set(KF5_MIN_VERSION "5.75.0")
set(QT_MIN_VERSION "5.15.0")

include(FeatureSummary)

################# KDE settings #################

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(KDEInstallDirs)
include(KDEClangFormat)
include(KDECMakeSettings)
include(ECMPoQmTools)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(QmlFormat.cmake)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX TELLY_SKOUT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/telly-skout-version.h
)

################# dependencies #################

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui QuickControls2 Sql)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS CoreAddons Config Crash I18n)

if (ANDROID)
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Kirigami2)
else()
    find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Widgets)
endif()

################# compiler #################

set(CMAKE_CXX_STANDARD 11)
if (MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-Wall -Wextra -Wconversion -Wsign-conversion -pedantic -Werror)
endif()

################# build and install #################

add_subdirectory(src)

install(PROGRAMS org.kde.telly-skout.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.telly-skout.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES telly-skout.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

################# format sources #################

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
add_custom_target(clang-format-always ALL DEPENDS ${ALL_CLANG_FORMAT_SOURCE_FILES})
add_dependencies(clang-format-always clang-format)

file(GLOB_RECURSE ALL_QML_FORMAT_SOURCE_FILES src/qml/*.qml)
qml_format(${ALL_QML_FORMAT_SOURCE_FILES})

add_custom_target(qml-format-always ALL DEPENDS ${ALL_QML_FORMAT_SOURCE_FILES})
add_dependencies(qml-format-always qml-format)
