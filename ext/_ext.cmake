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
if(EMSCRIPTEN)
    set(SOURCEPP_BUILD_WITH_THREADS OFF CACHE INTERNAL "" FORCE)
endif()
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/sourcepp")

# Qt
if(MARETF_BUILD_GUI OR MARETF_BUILD_THUMBNAILER)
    if(WIN32 AND NOT DEFINED QT_BASEDIR)
        message(FATAL_ERROR "Please define your Qt install dir with -DQT_BASEDIR=\"C:/your/qt6/here\"")
    endif()

    if(DEFINED QT_BASEDIR)
        string(REPLACE "\\" "/" QT_BASEDIR "${QT_BASEDIR}")

        # Add it to the prefix path so find_package can find it
        list(APPEND CMAKE_PREFIX_PATH "${QT_BASEDIR}")
        set(QT_INCLUDE "${QT_BASEDIR}/include")
        message(STATUS "Using ${QT_INCLUDE} as the Qt include directory")
    endif()

    if(EMSCRIPTEN)
        set(QT_WASM_INITIAL_MEMORY "128MB" CACHE INTERNAL "" FORCE)
    endif()

    # CMake has an odd policy that links a special link lib for Qt on newer versions of CMake
    cmake_policy(SET CMP0020 NEW)

    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network LinguistTools)

    get_target_property(QT_QMAKE_EXECUTABLE Qt::qmake IMPORTED_LOCATION)
    execute_process(COMMAND "${QT_QMAKE_EXECUTABLE}" -query QT_VERSION              OUTPUT_VARIABLE QT_VERSION          OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${QT_QMAKE_EXECUTABLE}" -query QT_INSTALL_TRANSLATIONS OUTPUT_VARIABLE QT_TRANSLATIONS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

    file(GLOB QT_I18N_QM_FILES "${QT_TRANSLATIONS_DIR}/qtbase_*.qm" "${QT_TRANSLATIONS_DIR}/qt_*.qm")
    if (QT_VERSION VERSION_GREATER_EQUAL "6.7.0")
        set_property(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/sourcepp" PROPERTY QT_EXCLUDE_FROM_TRANSLATION ON)
    endif()

    function(target_use_qt TARGET)
        set_target_properties(${TARGET} PROPERTIES AUTOMOC ON AUTORCC ON AUTOUIC ON)
        target_link_libraries(${TARGET} PRIVATE Qt::Core Qt::Gui Qt::Widgets Qt::Network)
        target_include_directories(${TARGET} PRIVATE "${QT_INCLUDE}" "${QT_INCLUDE}/QtCore" "${QT_INCLUDE}/QtGui" "${QT_INCLUDE}/QtWidgets" "${QT_INCLUDE}/QtNetwork")
        qt_add_resources(${TARGET} "${TARGET}_qt_translations" BASE "${QT_TRANSLATIONS_DIR}" PREFIX "/i18n" FILES ${QT_I18N_QM_FILES})
        if(EMSCRIPTEN)
            # todo: -gN and -gsource-map should be debug only
            target_compile_options(${PROJECT_NAME}_gui PRIVATE
                    -fexceptions
                    -fno-stack-protector
                    -fno-sanitize=all
                    -g3
                    -gsource-map)
            target_link_options(${TARGET} PRIVATE
                    --bind
                    -g3
                    -gsource-map
                    -sALLOW_MEMORY_GROWTH=1
                    -sASSERTIONS=1
                    -sASYNCIFY=1
                    -sASYNCIFY_IMPORTS=[QEventLoop::exec,QDialog::exec,QMessageBox::exec]
                    -sASYNCIFY_STACK_SIZE=65536
                    -sNO_DISABLE_EXCEPTION_CATCHING=1)
        endif()
    endfunction()

    # Copy these in
    list(APPEND QT_REQUIRED_ROOT_LIBS
            # Core
            "Core" "Gui" "Widgets" "Network"
            # Needed by plugins
            "DBus" "EglFSDeviceIntegration" "EglFsKmsSupport" "EglFsKmsGbmSupport" "WaylandClient" "WaylandEglClientHwIntegration" "WlShellIntegration" "XcbQpa")
    list(APPEND QT_REQUIRED_PLUGIN_DIRS
            "egldeviceintegrations" "networkinformation" "platforminputcontexts" "platforms" "platformthemes" "styles" "tls"
            "wayland-decoration-client" "wayland-graphics-integration-client" "wayland-shell-integration" "xcbglintegrations")

    if(WIN32)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set(QT_LIB_SUFFIX "d" CACHE STRING "" FORCE)
        else()
            set(QT_LIB_SUFFIX "" CACHE STRING "" FORCE)
        endif()

        foreach(QT_REQUIRED_ROOT_LIB ${QT_REQUIRED_ROOT_LIBS})
            if(EXISTS "${QT_BASEDIR}/bin/Qt6${QT_REQUIRED_ROOT_LIB}${QT_LIB_SUFFIX}.dll")
                file(COPY "${QT_BASEDIR}/bin/Qt6${QT_REQUIRED_ROOT_LIB}${QT_LIB_SUFFIX}.dll" DESTINATION "${CMAKE_BINARY_DIR}")
            endif()
        endforeach()

        foreach(QT_REQUIRED_PLUGIN_DIR ${QT_REQUIRED_PLUGIN_DIRS})
            if(EXISTS "${QT_BASEDIR}/plugins/${QT_REQUIRED_PLUGIN_DIR}")
                file(COPY "${QT_BASEDIR}/plugins/${QT_REQUIRED_PLUGIN_DIR}" DESTINATION "${CMAKE_BINARY_DIR}")
            endif()
        endforeach()
    elseif(UNIX AND DEFINED QT_BASEDIR)
        foreach(QT_REQUIRED_ROOT_LIB ${QT_REQUIRED_ROOT_LIBS})
            if(EXISTS "${QT_BASEDIR}/lib/libQt6${QT_REQUIRED_ROOT_LIB}.so")
                file(COPY "${QT_BASEDIR}/lib/libQt6${QT_REQUIRED_ROOT_LIB}.so" DESTINATION "${CMAKE_BINARY_DIR}/lib" FOLLOW_SYMLINK_CHAIN)
            endif()
        endforeach()

        foreach(QT_REQUIRED_PLUGIN_DIR ${QT_REQUIRED_PLUGIN_DIRS})
            if(EXISTS "${QT_BASEDIR}/plugins/${QT_REQUIRED_PLUGIN_DIR}")
                file(COPY "${QT_BASEDIR}/plugins/${QT_REQUIRED_PLUGIN_DIR}" DESTINATION "${CMAKE_BINARY_DIR}/plugins")
            endif()
        endforeach()

        # These end in a lot of different numbers depending on the Qt version
        function(copy_latest_icu_lib NAME)
            file(GLOB QT_ICU_LIBS "${QT_BASEDIR}/lib/libicu${NAME}.so.[0-9][0-9]" "${QT_BASEDIR}/lib/libicu${NAME}.so.[0-9][0-9][0-9]")
            if(QT_ICU_LIBS)
                list(SORT QT_ICU_LIBS)
                list(REVERSE QT_ICU_LIBS)
                list(GET QT_ICU_LIBS 0 QT_ICU_LIB)
                cmake_path(GET QT_ICU_LIB FILENAME QT_ICU_LIB_FILENAME)
                file(COPY "${QT_ICU_LIB}" DESTINATION "${CMAKE_BINARY_DIR}/lib" FOLLOW_SYMLINK_CHAIN)
            endif()
        endfunction()
        copy_latest_icu_lib("data")
        copy_latest_icu_lib("i18n")
        copy_latest_icu_lib("io")
        copy_latest_icu_lib("tu")
        copy_latest_icu_lib("uc")
    endif()
endif()
