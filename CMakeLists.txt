cmake_minimum_required(VERSION 3.14)

project(telly-scout)
set(PROJECT_VERSION "1.0")

set(KF5_MIN_VERSION "5.75.0")
set(QT_MIN_VERSION "5.15.0")

include(FeatureSummary)

################# set KDE specific information #################

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

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX TELLY_SCOUT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/telly-scout-version.h
)

################# Find dependencies #################

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui QuickControls2 Sql Widgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS CoreAddons Config I18n)


################# Enable C++11 features for clang and gcc #################

set(CMAKE_CXX_STANDARD 11)

################# build and install #################

add_subdirectory(src)

install(PROGRAMS org.kde.telly-scout.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.telly-scout.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES telly-scout.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

add_custom_target(clang-format-always ALL DEPENDS ${ALL_CLANG_FORMAT_SOURCE_FILES})
add_dependencies(clang-format-always clang-format)
