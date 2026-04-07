add_executable(${PROJECT_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/MareTF.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Common.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common/EnumMappings.h")
cs_configure_target(${PROJECT_NAME} LOGO MANIFEST)

target_compile_definitions(${PROJECT_NAME} PUBLIC MARETF_CLI)
if(MARETF_BUILD_FOR_STRATA_SOURCE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MARETF_BUILD_FOR_STRATA_SOURCE)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/cli" "${CMAKE_CURRENT_SOURCE_DIR}/src/common")

target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse efsw-static indicators::indicators sourcepp::kvpp sourcepp::vtfpp)
