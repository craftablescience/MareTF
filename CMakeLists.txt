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

# initial VS integration
set(CMAKE_FOLDER "Thirdparty" CACHE INTERNAL "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# set common compile options
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH $ORIGIN)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckIPOSupported)
option(MARETF_USE_LTO "Compile ${PROJECT_NAME_PRETTY} with LTO/IPO" OFF)
if(MARETF_USE_LTO)
    check_ipo_supported(RESULT MARETF_USE_LTO)
endif()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${MARETF_USE_LTO})

# macro to set up many things at once for a given target
function(maretf_configure_target TARGET)
    # Define DEBUG macro
    target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

    # Set optimization flags
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        # Build with debug friendly optimizations and debug symbols (MSVC defaults are fine)
        if(UNIX OR MINGW)
            target_compile_options(${TARGET} PRIVATE -Og -g)
        endif()
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MSVC)
            target_compile_options(${TARGET} PRIVATE -fno-limit-debug-info)
        endif()
    else()
        # Build with optimizations and don't omit stack pointer for debugging (MSVC defaults are fine)
        if(UNIX OR MINGW)
            target_compile_options(${TARGET} PRIVATE -O2 -fno-omit-frame-pointer)
        endif()
    endif()

    if(WIN32 AND MSVC)
        get_target_property(TARGET_TYPE ${TARGET} TYPE)
        get_target_property(TARGET_IS_GUI ${TARGET} WIN32_EXECUTABLE)

        # Organize VS solution into folders
        if(TARGET_TYPE STREQUAL "EXECUTABLE")
            set_target_properties(${TARGET} PROPERTIES FOLDER "Executables")
        else()
            set_target_properties(${TARGET} PROPERTIES FOLDER "Libraries")
        endif()

        # Create PDBs in release
        if((TARGET_TYPE STREQUAL "SHARED_LIBRARY") OR (TARGET_TYPE STREQUAL "EXECUTABLE"))
            target_compile_options(
                    ${TARGET} PRIVATE
                    "$<$<CONFIG:Release>:/Zi>")
            target_link_options(
                    ${TARGET} PRIVATE
                    "$<$<CONFIG:Release>:/DEBUG>"
                    "$<$<CONFIG:Release>:/OPT:REF>"
                    "$<$<CONFIG:Release>:/OPT:ICF>")
        endif()

        # Add an icon and manifest to the executable
        if(TARGET_TYPE STREQUAL "EXECUTABLE")
            target_sources(
                    ${TARGET} PRIVATE
                    "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.rc"
                    "${CMAKE_CURRENT_SOURCE_DIR}/res/program.manifest")
        endif()

        # Don't show the console when running the executable
        if(TARGET_IS_GUI)
            target_link_options(
                    ${TARGET} PRIVATE
                    "/ENTRY:mainCRTStartup")
        endif()
    endif()
endfunction()

# external libraries
include("${CMAKE_CURRENT_SOURCE_DIR}/ext/_ext.cmake")

# maretf
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h")
add_executable(${PROJECT_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
maretf_configure_target(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse sourcepp::kvpp sourcepp::vtfpp)

# maretf_gui
add_executable(${PROJECT_NAME}_gui
        "${CMAKE_CURRENT_SOURCE_DIR}/res/gui/res.qrc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MareTF.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/MareTF.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
maretf_configure_target(${PROJECT_NAME}_gui)
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
