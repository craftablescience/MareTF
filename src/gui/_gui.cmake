set(${PROJECT_NAME}_gui_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/res/gui/attribution.qrc"
        "${CMAKE_CURRENT_SOURCE_DIR}/res/gui/res.qrc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h"
        "${CMAKE_CURRENT_LIST_DIR}/dialogs/QMareCreateTextureDialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dialogs/QMareCreateTextureDialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/dialogs/QMareCreditsDialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dialogs/QMareCreditsDialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/utility/QMareDiscordPresence.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/utility/QMareDiscordPresence.h"
        "${CMAKE_CURRENT_LIST_DIR}/utility/QMareOptions.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/utility/QMareOptions.h"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareComboBox.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareComboBox.h"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareFlagsWidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareFlagsWidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareMiddleClickTabWidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareMiddleClickTabWidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareSpinBox.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareSpinBox.h"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareTextureWidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/widgets/QMareTextureWidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/windows/QMareEmptyWindow.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/windows/QMareEmptyWindow.h"
        "${CMAKE_CURRENT_LIST_DIR}/windows/QMareTextureWindow.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/windows/QMareTextureWindow.h"
        "${CMAKE_CURRENT_LIST_DIR}/MareTF.cpp")
add_executable(${PROJECT_NAME}_gui WIN32 MACOSX_BUNDLE ${${PROJECT_NAME}_gui_SOURCES})
cs_configure_target(${PROJECT_NAME}_gui LOGO MANIFEST)

if(MARETF_BUILD_FOR_STRATA_SOURCE)
    target_compile_definitions(${PROJECT_NAME}_gui PUBLIC MARETF_BUILD_FOR_STRATA_SOURCE)
endif()

target_include_directories(${PROJECT_NAME}_gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/cli" "${CMAKE_CURRENT_SOURCE_DIR}/src/common" "${CMAKE_CURRENT_SOURCE_DIR}/src/gui")

target_link_libraries(${PROJECT_NAME}_gui PRIVATE argparse::argparse discord-rpc efsw-static indicators::indicators sourcepp::kvpp sourcepp::vtfpp)

file(GLOB ${PROJECT_NAME}_gui_I18N_TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/i18n/${PROJECT_NAME}_*.ts")
qt_add_translations(${PROJECT_NAME}_gui
        TS_FILES ${${PROJECT_NAME}_gui_I18N_TS_FILES}
        RESOURCE_PREFIX "/i18n"
        SOURCES ${${PROJECT_NAME}_SOURCES})
target_use_qt(${PROJECT_NAME}_gui)
