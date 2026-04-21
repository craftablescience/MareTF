if(WIN32)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/windows/MareTF.def.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/windows/generated/MareTF.def")
    qt_add_library(${PROJECT_NAME}_thumbnailer SHARED
            "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/windows/generated/MareTF.def"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/windows/MareTF.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/CommonThumbnailer.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/CommonThumbnailer.h")
    target_link_libraries(${PROJECT_NAME}_thumbnailer PRIVATE Shlwapi)
else()
    qt_add_executable(${PROJECT_NAME}_thumbnailer
            "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/linux/MareTF.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/CommonThumbnailer.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer/CommonThumbnailer.h")
    target_link_libraries(${PROJECT_NAME}_thumbnailer PRIVATE argparse::argparse)
endif()
cs_configure_target(${PROJECT_NAME}_thumbnailer)

target_include_directories(${PROJECT_NAME}_thumbnailer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/common" "${CMAKE_CURRENT_SOURCE_DIR}/src/thumbnailer")

target_link_libraries(${PROJECT_NAME}_thumbnailer PRIVATE sourcepp::vtfpp)

target_use_qt(${PROJECT_NAME}_thumbnailer)
