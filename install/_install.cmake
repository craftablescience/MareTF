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
        if(MARETF_SYSTEM_INSTALL)
            install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
        elseif(CPACK_GENERATOR STREQUAL "AppImage")
            # Workaround because AppImage is making a "maretf" symlink to the licenses folder(???)
            install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}"
                    DESTINATION .
                    RENAME "maretf_cli")
        else()
            install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
        endif()
    endif()

    if(MARETF_BUILD_GUI)
        if(MARETF_SYSTEM_INSTALL)
            install(TARGETS ${PROJECT_NAME}_gui RUNTIME DESTINATION "bin")
        else()
            install(TARGETS ${PROJECT_NAME}_gui RUNTIME DESTINATION .)
        endif()

        # Desktop file
        if(MARETF_SYSTEM_INSTALL)
            configure_file(
                    "${CMAKE_CURRENT_LIST_DIR}/linux/desktop_system.in"
                    "${CMAKE_CURRENT_LIST_DIR}/linux/generated/${PROJECT_NAME}.desktop")
        else()
            configure_file(
                    "${CMAKE_CURRENT_LIST_DIR}/linux/desktop.in"
                    "${CMAKE_CURRENT_LIST_DIR}/linux/generated/${PROJECT_NAME}.desktop")
        endif()
        install(FILES "${CMAKE_CURRENT_LIST_DIR}/linux/generated/${PROJECT_NAME}.desktop"
                DESTINATION "share/applications")

        # Program icon
        if(CPACK_GENERATOR STREQUAL "AppImage")
            # Another AppImage hack hooray
            install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.png"
                    DESTINATION .
                    RENAME "${PROJECT_NAME}.png")
        else()
            install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.png"
                    DESTINATION "share/icons/hicolor/512x512/apps"
                    RENAME "${PROJECT_NAME}.png")
        endif()
    endif()

    if(MARETF_BUILD_THUMBNAILER)
        if(MARETF_SYSTEM_INSTALL)
            install(TARGETS ${PROJECT_NAME}_thumbnailer RUNTIME DESTINATION "bin")
        else()
            install(TARGETS ${PROJECT_NAME}_thumbnailer RUNTIME DESTINATION .)
        endif()

        # Thumbnailer file
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/thumbnailer.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/generated/${PROJECT_NAME}.thumbnailer")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/generated/${PROJECT_NAME}.thumbnailer"
                DESTINATION "share/thumbnailers")
    endif()

    if(MARETF_BUILD_GUI OR MARETF_BUILD_THUMBNAILER)
        if(MARETF_SYSTEM_INSTALL)
            # Use system Qt, no install rules
        elseif(DEFINED QT_BASEDIR)
            # Install Qt libraries and plugins
            install(DIRECTORY "${CMAKE_BINARY_DIR}/lib" "${CMAKE_BINARY_DIR}/plugins"
                    DESTINATION .
                    FILES_MATCHING PATTERN "*.so*")
        endif()

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
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
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
        set(CPACK_NSIS_URL_INFO_ABOUT "${PROJECT_HOMEPAGE_URL}")
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
    elseif(CPACK_GENERATOR MATCHES "(7Z.*)|(STGZ|TAR|TBZ2|TGZ|TXZ|TZST|TZ)|(ZIP.*)")
        # Supported :3
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
    elseif(CPACK_GENERATOR MATCHES "(7Z.*)|(STGZ|TAR|TBZ2|TGZ|TXZ|TZST|TZ)|(ZIP.*)")
        # Supported :3
    else()
        message(WARNING "CPACK_GENERATOR is unset, or set to an unrecognized generator.")
    endif()
endif()
include(CPack)
