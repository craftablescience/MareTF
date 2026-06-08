cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# set up project
project(maretf
        DESCRIPTION "A utility to create, edit, and display every type of VTF file ever made."
        VERSION 0.12.0
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

# common documents and config
if(MARETF_BUILD_CLI OR MARETF_BUILD_GUI OR MARETF_BUILD_THUMBNAILER)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CREDITS" "${CMAKE_BINARY_DIR}/CREDITS" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/LICENSE" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h")
endif()

# maretf
if(MARETF_BUILD_CLI)
    cs_include_directory(src/cli)
endif()

# maretf_gui
if(MARETF_BUILD_GUI)
    cs_include_directory(src/gui)
endif()

# maretf_thumbnailer
if(MARETF_BUILD_THUMBNAILER)
    cs_include_directory(src/thumbnailer)
endif()

# installer
if(MARETF_BUILD_INSTALLER)
    cs_include_directory(install)
endif()

# VS setup
if(WIN32 AND MARETF_BUILD_GUI)
    set_target_properties(
            ${PROJECT_NAME}_gui PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${QT_BASEDIR}/bin;")
endif()
