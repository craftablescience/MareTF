cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# set up project
project(maretf
        DESCRIPTION "A utility to create, edit, and display every type of VTF file ever made."
        VERSION 0.10.2
        HOMEPAGE_URL "https://github.com/craftablescience/MareTF")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add helpers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers")
include(CS_All)

# extra metadata variables
set(PROJECT_NAME_PRETTY "MareTF" CACHE STRING "" FORCE)
cs_version_pretty() # PROJECT_VERSION_PRETTY
set(PROJECT_ORGANIZATION_NAME "craftablescience" CACHE STRING "" FORCE)

# options
option(MARETF_BUILD_FOR_STRATA_SOURCE "Build ${PROJECT_NAME_PRETTY} for Strata Source games" OFF)
option(MARETF_BUILD_CLI               "Build ${PROJECT_NAME_PRETTY} CLI"                      ON)
option(MARETF_BUILD_GUI               "Build ${PROJECT_NAME_PRETTY} GUI"                      ON)
option(MARETF_BUILD_THUMBNAILER       "Build ${PROJECT_NAME_PRETTY} thumbnailer"              ON)
option(MARETF_BUILD_INSTALLER         "Build ${PROJECT_NAME_PRETTY} installer"               OFF)

# initial VS integration
cs_setup_vs_defaults(STARTUP_PROJECT "${PROJECT_NAME}")

# set common compile options
option(MARETF_USE_LTO "Compile ${PROJECT_NAME_PRETTY} with LTO/IPO" OFF)
cs_configure_defaults(MARETF_USE_LTO)

# external libraries
cs_include_directory(ext)

# common config
if(MARETF_BUILD_CLI OR MARETF_BUILD_GUI OR MARETF_BUILD_THUMBNAILER)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h")
endif()

# maretf
if(MARETF_BUILD_CLI)
    add_executable(${PROJECT_NAME}
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
    cs_configure_target(${PROJECT_NAME} LOGO MANIFEST)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MARETF_CLI)
    if(MARETF_BUILD_FOR_STRATA_SOURCE)
        target_compile_definitions(${PROJECT_NAME} PUBLIC MARETF_BUILD_FOR_STRATA_SOURCE)
    endif()
    target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/cli" "${CMAKE_CURRENT_SOURCE_DIR}/src/common")
    target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse efsw-static indicators::indicators sourcepp::kvpp sourcepp::vtfpp)
endif()

# maretf_gui
if(MARETF_BUILD_GUI)
    add_executable(${PROJECT_NAME}_gui WIN32 MACOSX_BUNDLE
            "${CMAKE_CURRENT_SOURCE_DIR}/res/gui/res.qrc"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/dialogs/QMareCreateTextureDialog.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/dialogs/QMareCreateTextureDialog.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/dialogs/QMareCredits.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/dialogs/QMareCredits.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/utility/QMareDiscordPresence.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/utility/QMareDiscordPresence.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/utility/QMareOptions.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/utility/QMareOptions.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/QMareComboBox.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/QMareComboBox.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/QMareFlagsWidget.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/QMareFlagsWidget.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/QMareSpinBox.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/QMareSpinBox.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/QMareTextureWidget.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/widgets/QMareTextureWidget.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/windows/QMareEmptyWindow.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/windows/QMareEmptyWindow.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/windows/QMareTextureWindow.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/windows/QMareTextureWindow.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MareTF.cpp")
    cs_configure_target(${PROJECT_NAME}_gui LOGO MANIFEST)
    if(MARETF_BUILD_FOR_STRATA_SOURCE)
        target_compile_definitions(${PROJECT_NAME}_gui PUBLIC MARETF_BUILD_FOR_STRATA_SOURCE)
    endif()
    target_include_directories(${PROJECT_NAME}_gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/cli" "${CMAKE_CURRENT_SOURCE_DIR}/src/common" "${CMAKE_CURRENT_SOURCE_DIR}/src/gui")
    target_use_qt(${PROJECT_NAME}_gui)
    target_link_libraries(${PROJECT_NAME}_gui PRIVATE argparse::argparse discord-rpc efsw-static indicators::indicators sourcepp::kvpp sourcepp::vtfpp)
endif()

# maretf_thumbnailer
if(MARETF_BUILD_THUMBNAILER)
    if(WIN32)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/windows/MareTF.def.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/windows/generated/MareTF.def")
        add_library(${PROJECT_NAME}_thumbnailer SHARED
                "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/windows/generated/MareTF.def"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/windows/MareTF.cpp")
        target_link_libraries(${PROJECT_NAME}_thumbnailer PRIVATE Shlwapi)
    else()
        add_executable(${PROJECT_NAME}_thumbnailer "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/linux/MareTF.cpp")
        target_link_libraries(${PROJECT_NAME}_thumbnailer PRIVATE argparse::argparse)
    endif()
    cs_configure_target(${PROJECT_NAME}_thumbnailer)
    target_include_directories(${PROJECT_NAME}_thumbnailer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/common")
    target_use_qt(${PROJECT_NAME}_thumbnailer)
    target_link_libraries(${PROJECT_NAME}_thumbnailer PRIVATE sourcepp::vtfpp)
endif()

# installer
if(MARETF_BUILD_INSTALLER)
    cs_include_directory(install)
endif()

# copy the license
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/LICENSE" COPYONLY)

# VS setup
if(WIN32 AND MARETF_BUILD_GUI)
    set_target_properties(
            ${PROJECT_NAME}_gui PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${QT_BASEDIR}/bin;")
endif()
