cmake_minimum_required(VERSION 3.13)
project(lvgl)

option(LV_USE_FREETYPE "Use freetype lib" ON)
option(USE_SIMULATOR "Use SDL2 simulator" ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)


set(CMAKE_C_STANDARD 99)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set(LV_CONF_BUILD_DISABLE_EXAMPLES 1)
set(LV_CONF_BUILD_DISABLE_DEMOS 1)

add_compile_definitions($<$<BOOL:${USE_SIMULATOR}>:USE_SIMULATOR=1>)

# set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

if(USE_SIMULATOR)
    find_package(SDL2 REQUIRED SDL2)
endif(USE_SIMULATOR)


add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})


add_subdirectory(main/src/widgets/terminal)

add_executable(main
    main/src/main.c
    main/src/term_io.c 
    main/src/mouse_cursor_icon.c
    main/src/config.c
    )

target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_include_directories(main PUBLIC)
message("SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
target_link_libraries(main lvgl lvgl::thorvg lvgl_terminal ${SDL2_LIBRARIES} m pthread)




add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)



if(LV_USE_FREETYPE)

    # The NEW behavior for this policy is to do nothing when a normal variable of the same name exists.
    # The normal variable is not removed.
    # The cache entry is not created or updated and is ignored if it exists.
    set(CMAKE_POLICY_DEFAULT_CMP0077 "NEW")
    set(FT_DISABLE_ZLIB TRUE)
    set(FT_DISABLE_BZIP2 TRUE)
    set(FT_DISABLE_PNG TRUE)
    set(FT_DISABLE_HARFBUZZ TRUE)
    set(FT_DISABLE_BROTLI TRUE)
    add_subdirectory(modules/freetype-2.13.2/)
    set(CMAKE_POLICY_DEFAULT_CMP0077 "OLD")

    
    get_target_property(FREETYPE_INCLUDE_DIRS freetype INCLUDE_DIRECTORIES)

    target_link_libraries(main freetype)
    target_include_directories(lvgl PUBLIC ${FREETYPE_INCLUDE_DIRS})
endif(LV_USE_FREETYPE)

add_subdirectory(modules/cargs)
target_link_libraries(main cargs)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(lvgl PRIVATE
        -pedantic-errors
        -Wall
        -Wclobbered
        -Wdeprecated
        -Wdouble-promotion
        -Wempty-body
        -Wextra
        -Wformat-security
        -Wmaybe-uninitialized
        # -Wmissing-prototypes
        -Wpointer-arith
        -Wmultichar
        -Wno-pedantic # ignored for now, we convert functions to pointers for propertis table.
        -Wreturn-type
        -Wshadow
        -Wshift-negative-value
        -Wsizeof-pointer-memaccess
        -Wtype-limits
        -Wundef
        -Wuninitialized
        -Wunreachable-code
        -Wfloat-conversion
        -Wstrict-aliasing
    )

    target_compile_options(main PRIVATE -fsanitize=address,leak,undefined -Wall -pedantic-errors -Wno-pedantic)
    target_link_options(main PRIVATE -fsanitize=address,leak,undefined)
    # target_link_options(main PRIVATE -static)
else()

target_compile_options(main PRIVATE
        -Wno-maybe-uninitialized
    )

target_compile_options(vterm PRIVATE
    -Wno-maybe-uninitialized
)

target_compile_options(main PRIVATE -Wno-error)

# if(NOT BUILD_SHARED_LIBS)
#     target_link_options(main PRIVATE -static)    
# endif(BUILD_SHARED_LIBS)


endif()