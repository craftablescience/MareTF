# global overrides
set(BUILD_SHARED_LIBS OFF) # here because of efsw, the little scamp

# argparse
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/argparse")

# discord
if(MARETF_BUILD_GUI)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/discord")
endif()

# efsw
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/efsw")

# indicators
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/indicators")

# sourcepp
set(SOURCEPP_LIBS_START_ENABLED OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_KVPP            ON CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_VTFPP           ON CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/sourcepp")

if(MARETF_BUILD_GUI)
    # Qt
    if(WIN32 AND NOT DEFINED QT_BASEDIR)
        message(FATAL_ERROR "Please define your QT install dir with -DQT_BASEDIR=\"C:/your/qt6/here\"")
    endif()

    if(DEFINED QT_BASEDIR)
        string(REPLACE "\\" "/" QT_BASEDIR "${QT_BASEDIR}")

        # Add it to the prefix path so find_package can find it
        list(APPEND CMAKE_PREFIX_PATH "${QT_BASEDIR}")
        set(QT_INCLUDE "${QT_BASEDIR}/include")
        message(STATUS "Using ${QT_INCLUDE} as the Qt include directory")
    endif()

    # CMake has an odd policy that links a special link lib for Qt on newer versions of CMake
    cmake_policy(SET CMP0020 NEW)

    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

    function(target_use_qt TARGET)
        set_target_properties(${TARGET} PROPERTIES AUTOMOC ON AUTORCC ON AUTOUIC ON)
        target_link_libraries(${TARGET} PRIVATE Qt::Core Qt::Gui Qt::Widgets)
        target_include_directories(${TARGET} PRIVATE "${QT_INCLUDE}" "${QT_INCLUDE}/QtCore" "${QT_INCLUDE}/QtGui" "${QT_INCLUDE}/QtWidgets")
    endfunction()

    # Copy these in
    if(WIN32)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set(QT_LIB_SUFFIX "d" CACHE STRING "" FORCE)
        else()
            set(QT_LIB_SUFFIX "" CACHE STRING "" FORCE)
        endif()
        configure_file("${QT_BASEDIR}/bin/Qt6Core${QT_LIB_SUFFIX}.dll"                        "${CMAKE_BINARY_DIR}/Qt6Core${QT_LIB_SUFFIX}.dll"                    COPYONLY)
        configure_file("${QT_BASEDIR}/bin/Qt6Gui${QT_LIB_SUFFIX}.dll"                         "${CMAKE_BINARY_DIR}/Qt6Gui${QT_LIB_SUFFIX}.dll"                     COPYONLY)
        configure_file("${QT_BASEDIR}/bin/Qt6Widgets${QT_LIB_SUFFIX}.dll"                     "${CMAKE_BINARY_DIR}/Qt6Widgets${QT_LIB_SUFFIX}.dll"                 COPYONLY)
        configure_file("${QT_BASEDIR}/plugins/platforms/qwindows${QT_LIB_SUFFIX}.dll"         "${CMAKE_BINARY_DIR}/platforms/qwindows${QT_LIB_SUFFIX}.dll"         COPYONLY)
        configure_file("${QT_BASEDIR}/plugins/styles/qmodernwindowsstyle${QT_LIB_SUFFIX}.dll" "${CMAKE_BINARY_DIR}/styles/qmodernwindowsstyle${QT_LIB_SUFFIX}.dll" COPYONLY)
    endif()
endif()
