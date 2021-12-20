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
include(KDEInstallDirs)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX TELLY_SKOUT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/telly-skout-version.h
)

################# dependencies #################

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui QuickControls2 Sql Widgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS CoreAddons Config I18n)

################# compiler #################

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -Wextra -pedantic -Werror)

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

find_program(QML_FORMAT_EXECUTABLE qmlformat)
function(QML_FORMAT)
    if (TARGET qml-format)
        message(WARNING "the qml_format function was already called")
        return()
    endif()

    # add target without specific commands first, we add the real calls file-per-file to avoid command line length issues
    add_custom_target(qml-format COMMENT "Formatting qml files in ${CMAKE_CURRENT_SOURCE_DIR} with ${QML_FORMAT_EXECUTABLE}...")

    # run qml-format only if available, else signal the user what is missing
    if(QML_FORMAT_EXECUTABLE)
        get_filename_component(_binary_dir ${CMAKE_BINARY_DIR} REALPATH)
        foreach(_file ${ARGV})
            # check if the file is inside the build directory => ignore such files
            get_filename_component(_full_file_path ${_file} REALPATH)
            string(FIND ${_full_file_path} ${_binary_dir} _index)
            if(NOT _index EQUAL 0)
                add_custom_command(TARGET qml-format
                    COMMAND
                        ${QML_FORMAT_EXECUTABLE}
                        -i
                        ${_full_file_path}
                    WORKING_DIRECTORY
                        ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT
                        "Formatting ${_full_file_path}..."
                    )
            endif()
        endforeach()
    else()
        add_custom_command(TARGET qml-format
            COMMAND
                ${CMAKE_COMMAND} -E echo "Could not set up the qml-format target as the qml-format executable is missing."
            )
    endif()
endfunction()

file(GLOB_RECURSE ALL_QML_FORMAT_SOURCE_FILES src/qml/*.qml)
qml_format(${ALL_QML_FORMAT_SOURCE_FILES})

add_custom_target(qml-format-always ALL DEPENDS ${ALL_QML_FORMAT_SOURCE_FILES})
add_dependencies(qml-format-always qml-format)
