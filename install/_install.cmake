# Configuration stuff
if(WIN32)
    # Qt
    install(IMPORTED_RUNTIME_ARTIFACTS
            Qt6::Core Qt6::Gui Qt6::Widgets
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .)

    install(FILES "${QT_BASEDIR}/plugins/platforms/qwindows${QT_LIB_SUFFIX}.dll"
            DESTINATION platforms)

    install(FILES "${QT_BASEDIR}/plugins/styles/qwindowsvistastyle${QT_LIB_SUFFIX}.dll"
            DESTINATION styles)

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
elseif(UNIX)
    # Use system Qt - no install rules

    # Celestia Medium Redux font
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/gui/CelestiaMediumRedux.ttf"
            DESTINATION "share/fonts/truetype")

    # Desktop file
    configure_file(
            "${CMAKE_CURRENT_LIST_DIR}/linux/desktop.in"
            "${CMAKE_CURRENT_LIST_DIR}/linux/generated/${PROJECT_NAME}.desktop")
    install(FILES "${CMAKE_CURRENT_LIST_DIR}/linux/generated/${PROJECT_NAME}.desktop"
            DESTINATION "share/applications")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.png"
            DESTINATION "share/pixmaps"
            RENAME "${PROJECT_NAME}.png")

    # MIME type info
    configure_file(
            "${CMAKE_CURRENT_LIST_DIR}/linux/mime-type.xml.in"
            "${CMAKE_CURRENT_LIST_DIR}/linux/generated/mime-type.xml")
    install(FILES "${CMAKE_CURRENT_LIST_DIR}/linux/generated/mime-type.xml"
            DESTINATION "share/mime/packages"
            RENAME "${PROJECT_NAME}.xml")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.png"
            DESTINATION "share/icons/hicolor/512x512/mimetypes"
            RENAME "image-x-vtf.png")
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
set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME_PRETTY})
set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if(WIN32)
    install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_gui RUNTIME DESTINATION .)

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
    file(READ "${CMAKE_CURRENT_LIST_DIR}/win/generated/InstallCommands.nsh" CPACK_NSIS_EXTRA_INSTALL_COMMANDS)
    file(READ "${CMAKE_CURRENT_LIST_DIR}/win/generated/UninstallCommands.nsh" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/install/win") # NSIS.template.in, NSIS.InstallOptions.ini.in
else()
    install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_gui RUNTIME DESTINATION bin)

    if(NOT (CPACK_GENERATOR STREQUAL "DEB"))
        message(AUTHOR_WARNING "CPack generator must be DEB! Setting generator to DEB...")
        set(CPACK_GENERATOR "DEB" CACHE INTERNAL "" FORCE)
    endif()
    set(CPACK_STRIP_FILES ON)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libxcb-cursor0, libqt6core6, libqt6gui6, libqt6widgets6")
    set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
endif()
include(CPack)
