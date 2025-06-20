cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "" FORCE)
endif()

# set up project
project(maretf
        DESCRIPTION "A work in progress command-line utility to work with VTF files."
        VERSION 0.6.1
        HOMEPAGE_URL "https://github.com/craftablescience/MareTF")
set(PROJECT_NAME_PRETTY "MareTF")
set(PROJECT_ORGANIZATION_NAME "craftablescience" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# options
option(MARETF_BUILD_GUI       "Build MareTF GUI"       OFF)
option(MARETF_BUILD_INSTALLER "Build MareTF installer" OFF)

# add helpers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers")
include(CS_All)

# initial VS integration
cs_setup_vs_defaults(STARTUP_PROJECT "${PROJECT_NAME}")

# set common compile options
option(MARETF_USE_LTO "Compile ${PROJECT_NAME_PRETTY} with LTO/IPO" OFF)
cs_configure_defaults(MARETF_USE_LTO)

# external libraries
cs_include_directory(ext)

# maretf
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h")
add_executable(${PROJECT_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
cs_configure_target(${PROJECT_NAME} LOGO MANIFEST)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse efsw-static indicators::indicators sourcepp::kvpp sourcepp::vtfpp)

# maretf_gui
if(MARETF_BUILD_GUI)
    add_executable(${PROJECT_NAME}_gui WIN32 MACOSX_BUNDLE
            "${CMAKE_CURRENT_SOURCE_DIR}/res/gui/res.qrc"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MareTF.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MareTF.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
    cs_configure_target(${PROJECT_NAME}_gui LOGO MANIFEST)
    target_use_qt(${PROJECT_NAME}_gui)
    target_link_libraries(${PROJECT_NAME}_gui PRIVATE discord-rpc sourcepp::vtfpp)
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
