cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "" FORCE)
endif()

# set up project
project(maretf
        DESCRIPTION "A work in progress command-line utility to work with VTF files."
        VERSION 0.2.0
        HOMEPAGE_URL "https://github.com/craftablescience/MareTF")
set(PROJECT_NAME_PRETTY "MareTF")
set(PROJECT_ORGANIZATION_NAME "craftablescience" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

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

        # Create PDBs in release
        if(MSVC AND ((TARGET_TYPE STREQUAL "SHARED_LIBRARY") OR (TARGET_TYPE STREQUAL "EXECUTABLE")))
            target_compile_options(
                    ${TARGET} PRIVATE
                    "$<$<CONFIG:Release>:/Zi>")
            target_link_options(
                    ${TARGET} PRIVATE
                    "$<$<CONFIG:Release>:/DEBUG>"
                    "$<$<CONFIG:Release>:/OPT:REF>"
                    "$<$<CONFIG:Release>:/OPT:ICF>")
        endif()

        # Add an icon to the executable
        if(TARGET_TYPE STREQUAL "EXECUTABLE")
            target_sources(
                    ${TARGET} PRIVATE
                    "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.rc")
        endif()

        # Don't show the console when running the executable
        if(MSVC AND TARGET_IS_GUI)
            target_link_options(
                    ${TARGET} PRIVATE
                    "/ENTRY:mainCRTStartup")
        endif()
    endif()
endfunction()

# argparse
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/argparse")

# sourcepp
set(SOURCEPP_LIBS_START_ENABLED OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_VTFPP           ON CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/sourcepp")

# maretf
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h")
add_executable(${PROJECT_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
maretf_configure_target(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse sourcepp::vtfpp)

# installer
option(MARETF_BUILD_INSTALLER "Build installer" OFF)
if(MARETF_BUILD_INSTALLER)
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME_PRETTY})
    set(CPACK_PACKAGE_VENDOR ${PROJECT_ORGANIZATION_NAME})
    set(CPACK_PACKAGE_CONTACT "lauralewisdev@gmail.com")
    set(CPACK_PACKAGE_DESCRIPTION ${CMAKE_PROJECT_DESCRIPTION})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME_PRETTY})
    if(WIN32)
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

        if(NOT (CPACK_GENERATOR STREQUAL "NSIS"))
            message(AUTHOR_WARNING "CPack generator must be NSIS! Setting generator to NSIS...")
            set(CPACK_GENERATOR "NSIS" CACHE INTERNAL "" FORCE)
        endif()
        set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        set(CPACK_NSIS_MODIFY_PATH ON)
        set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME_PRETTY})
        set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME_PRETTY})
        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.ico")
        set(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_NAME}.exe")
        set(CPACK_NSIS_URL_INFO_ABOUT "${CMAKE_PROJECT_HOMEPAGE_URL}")
        set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
    else()
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

        if(NOT (CPACK_GENERATOR STREQUAL "DEB"))
            message(AUTHOR_WARNING "CPack generator must be DEB! Setting generator to DEB...")
            set(CPACK_GENERATOR "DEB" CACHE INTERNAL "" FORCE)
        endif()
        set(CPACK_STRIP_FILES ON)
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
        set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
    endif()
    include(CPack)
endif()
