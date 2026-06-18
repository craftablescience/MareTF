# Configuration stuff
if(WIN32)
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/CREDITS"
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
            DESTINATION .)

    if(MARETF_BUILD_CLI)
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
    endif()

    if(MARETF_BUILD_GUI)
        install(TARGETS ${PROJECT_NAME}_gui RUNTIME DESTINATION .)

        # NSIS install commands
        configure_file(
                "${CMAKE_CURRENT_LIST_DIR}/win/InstallCommands.nsh.in"
                "${CMAKE_CURRENT_LIST_DIR}/win/generated/InstallCommands.nsh"
                @ONLY)

        # NSIS uninstall commands
        configure_file(
                "${CMAKE_CURRENT_LIST_DIR}/win/UninstallCommands.nsh.in"
                "${CMAKE_CURRENT_LIST_DIR}/win/generated/UninstallCommands.nsh"
                @ONLY)
    endif()

    if(MARETF_BUILD_THUMBNAILER)
        install(TARGETS ${PROJECT_NAME}_thumbnailer RUNTIME DESTINATION .)
    endif()

    if(MARETF_BUILD_GUI OR MARETF_BUILD_THUMBNAILER)
        # Qt
        install(IMPORTED_RUNTIME_ARTIFACTS
                Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .)

        install(FILES "${QT_BASEDIR}/plugins/platforms/qwindows${QT_LIB_SUFFIX}.dll"
                DESTINATION "platforms")

        if(EXISTS "${QT_BASEDIR}/plugins/styles/qmodernwindowsstyle${QT_LIB_SUFFIX}.dll")
            install(FILES "${QT_BASEDIR}/plugins/styles/qmodernwindowsstyle${QT_LIB_SUFFIX}.dll"
                    DESTINATION "styles")
        else()
            install(FILES "${QT_BASEDIR}/plugins/styles/qwindowsvistastyle${QT_LIB_SUFFIX}.dll"
                    DESTINATION "styles")
        endif()

        install(FILES
                "${QT_BASEDIR}/plugins/tls/qcertonlybackend${QT_LIB_SUFFIX}.dll"
                "${QT_BASEDIR}/plugins/tls/qschannelbackend${QT_LIB_SUFFIX}.dll"
                DESTINATION tls)
    endif()
elseif(UNIX)
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/CREDITS"
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
            DESTINATION "share/licenses/${PROJECT_NAME}")

    if(MARETF_BUILD_CLI)
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
    endif()

    if(MARETF_BUILD_GUI)
        install(TARGETS ${PROJECT_NAME}_gui RUNTIME DESTINATION "bin")

        # Desktop file and program icon
        if((CPACK_GENERATOR STREQUAL "DEB") OR (CPACK_GENERATOR STREQUAL "RPM"))
            configure_file(
                    "${CMAKE_CURRENT_LIST_DIR}/linux/desktop_sys.in"
                    "${CMAKE_CURRENT_LIST_DIR}/linux/generated/${PROJECT_NAME}.desktop")
            install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.png"
                    DESTINATION "share/icons/hicolor/512x512/apps"
                    RENAME "${PROJECT_NAME}.png")
        else()
            configure_file(
                    "${CMAKE_CURRENT_LIST_DIR}/linux/desktop.in"
                    "${CMAKE_CURRENT_LIST_DIR}/linux/generated/${PROJECT_NAME}.desktop")
            install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.png"
                    DESTINATION "."
                    RENAME "${PROJECT_NAME}.png")
        endif()
        install(FILES "${CMAKE_CURRENT_LIST_DIR}/linux/generated/${PROJECT_NAME}.desktop"
                DESTINATION "share/applications")
    endif()

    if(MARETF_BUILD_THUMBNAILER)
        install(TARGETS ${PROJECT_NAME}_thumbnailer RUNTIME DESTINATION "bin")

        # Thumbnailer file
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/thumbnailer.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/generated/${PROJECT_NAME}.thumbnailer")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/generated/${PROJECT_NAME}.thumbnailer"
                DESTINATION "share/thumbnailers")
    endif()

    if(MARETF_BUILD_GUI OR MARETF_BUILD_THUMBNAILER)
        # Use system Qt - no install rules

        # MIME type info
        configure_file(
                "${CMAKE_CURRENT_LIST_DIR}/linux/mime-type.xml.in"
                "${CMAKE_CURRENT_LIST_DIR}/linux/generated/mime-type.xml")
        install(FILES "${CMAKE_CURRENT_LIST_DIR}/linux/generated/mime-type.xml"
                DESTINATION "share/mime/packages"
                RENAME "${PROJECT_NAME}.xml")
    endif()
else()
    message(FATAL_ERROR "No install rules for selected platform.")
endif()

# CPack setup
set(CPACK_PACKAGE_NAME ${PROJECT_NAME_PRETTY})
set(CPACK_PACKAGE_VENDOR ${PROJECT_ORGANIZATION_NAME})
set(CPACK_PACKAGE_CONTACT "lauralewisdev@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION ${CMAKE_PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_MONOLITHIC_INSTALL ON)
if(MARETF_BUILD_GUI)
    set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME}_gui ${PROJECT_NAME_PRETTY})
endif()
set(CPACK_STRIP_FILES ON)
set(CPACK_THREADS 0)
set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if(WIN32)
    if((CPACK_GENERATOR STREQUAL "NSIS") OR (CPACK_GENERATOR STREQUAL "NSIS64"))
        set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        set(CPACK_NSIS_MODIFY_PATH ON)
        set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME_PRETTY})
        set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME_PRETTY})
        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.ico")
        if(MARETF_BUILD_CLI)
            set(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_NAME}.exe")
        elseif(MARETF_BUILD_GUI)
            set(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_NAME}_gui.exe")
        endif()
        set(CPACK_NSIS_URL_INFO_ABOUT "${CMAKE_PROJECT_HOMEPAGE_URL}")
        set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
        if(MARETF_BUILD_GUI)
            file(READ "${CMAKE_CURRENT_LIST_DIR}/win/generated/InstallCommands.nsh" CPACK_NSIS_EXTRA_INSTALL_COMMANDS)
            file(READ "${CMAKE_CURRENT_LIST_DIR}/win/generated/UninstallCommands.nsh" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)
            list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/install/win") # NSIS.template.in, NSIS.InstallOptions.ini.in
        endif()
        if(MARETF_BUILD_THUMBNAILER)
            set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait 'regsvr32 /s \\\"$INSTDIR\\\\${PROJECT_NAME}_thumbnailer.dll\\\"'")
            set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "ExecWait 'regsvr32 /u /s \\\"$INSTDIR\\\\${PROJECT_NAME}_thumbnailer.dll\\\"'")
        endif()
    else()
        message(WARNING "CPACK_GENERATOR is unset, or set to an unrecognized generator.")
    endif()
elseif(APPLE)
    message(FATAL_ERROR "${PROJECT_NAME_PRETTY} macOS install script does not exist.")
else()
    if(CPACK_GENERATOR STREQUAL "AppImage")
        set(CPACK_PACKAGE_ICON "${PROJECT_NAME}")
    elseif(CPACK_GENERATOR STREQUAL "DEB")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
        if(MARETF_BUILD_GUI)
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "libxcb-cursor0, libqt6core6 (>= 6.4.2), libqt6gui6 (>= 6.4.2), libqt6widgets6 (>= 6.4.2), libqt6network6 (>= 6.4.2)")
        endif()
        set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
    elseif(CPACK_GENERATOR STREQUAL "RPM")
        set(CPACK_RPM_PACKAGE_LICENSE "MIT")
        if(MARETF_BUILD_GUI)
            set(CPACK_RPM_PACKAGE_REQUIRES "libxcb, qt6-qtbase >= 6.4.2, qt6-qtbase-gui >= 6.4.2")
        endif()
        if(CMAKE_VERSION VERSION_LESS "3.31")
            set(CPACK_RPM_COMPRESSION_TYPE "xz")
        else()
            set(CPACK_RPM_COMPRESSION_TYPE "zstd")
        endif()
    else()
        message(WARNING "CPACK_GENERATOR is unset, or set to an unrecognized generator.")
    endif()
endif()
include(CPack)
