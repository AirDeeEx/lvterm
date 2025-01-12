cmake_minimum_required(VERSION 3.10)

add_library(lvgl_terminal
    lv_terminal.c
    )

add_subdirectory(libvterm)

#add_dependencies(lvgl_terminal libvterm)

#include_directories(${CMAKE_SOURCE_DIR}/main/src/libvterm/include/)
target_include_directories(lvgl_terminal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libvterm/include/)
target_link_libraries(lvgl_terminal lvgl m pthread vterm)