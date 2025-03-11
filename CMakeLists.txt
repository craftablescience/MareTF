cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "" FORCE)
endif()

# set up project
project(maretf
        DESCRIPTION "A work in progress command-line utility to work with VTF files."
        VERSION 0.4.0
        HOMEPAGE_URL "https://github.com/craftablescience/MareTF")
set(PROJECT_NAME_PRETTY "MareTF")
set(PROJECT_ORGANIZATION_NAME "craftablescience" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add helpers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers")
include(CS_All)

# initial VS integration
cs_setup_vs_defaults()

# set common compile options
option(MARETF_USE_LTO "Compile ${PROJECT_NAME_PRETTY} with LTO/IPO" OFF)
cs_configure_defaults(MARETF_USE_LTO)

# external libraries
include("${CMAKE_CURRENT_SOURCE_DIR}/ext/_ext.cmake")

# maretf
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h")
add_executable(${PROJECT_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
cs_configure_target(${PROJECT_NAME} LOGO MANIFEST)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse sourcepp::kvpp sourcepp::vtfpp)

# maretf_gui
add_executable(${PROJECT_NAME}_gui WIN32
        "${CMAKE_CURRENT_SOURCE_DIR}/res/gui/res.qrc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MareTF.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MareTF.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
cs_configure_target(${PROJECT_NAME}_gui LOGO MANIFEST)
target_link_libraries(${PROJECT_NAME}_gui PRIVATE Qt::Core Qt::Gui Qt::Widgets sourcepp::kvpp sourcepp::vtfpp)
target_include_directories(${PROJECT_NAME}_gui PRIVATE "${QT_INCLUDE}" "${QT_INCLUDE}/QtCore" "${QT_INCLUDE}/QtGui" "${QT_INCLUDE}/QtWidgets")

# installer
option(MARETF_BUILD_INSTALLER "Build installer" OFF)
if(MARETF_BUILD_INSTALLER)
    include("${CMAKE_CURRENT_SOURCE_DIR}/install/_install.cmake")
endif()

# VS setup
if(WIN32)
    set_property(
            DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}_gui")
    set_target_properties(
            ${PROJECT_NAME}_gui PROPERTIES
            VS_DEBUGGER_COMMAND "$<TARGET_FILE:${PROJECT_NAME}_gui>"
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
            VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${QT_BASEDIR}/bin;")
endif()
