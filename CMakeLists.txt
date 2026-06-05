cmake_minimum_required(VERSION 3.20)
project(raypoketrack VERSION 0.2.0 LANGUAGES C)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
endif()

set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_C_STANDARD 11)

include(FetchContent)

# raylib 6.0
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.zip)
FetchContent_MakeAvailable(raylib)

# CLAP headers (instruments + effects plugin standard)
FetchContent_Declare(clap URL https://github.com/free-audio/clap/archive/refs/tags/1.2.6.zip)
FetchContent_MakeAvailable(clap)

# TinySoundFont (single-header SF2 player)
set(TSF_HEADER "${CMAKE_BINARY_DIR}/tsf_include/tsf.h")
if(NOT EXISTS "${TSF_HEADER}")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tsf_include")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/schellingb/TinySoundFont/master/tsf.h"
        "${TSF_HEADER}"
        STATUS TSF_STATUS
    )
    list(GET TSF_STATUS 0 TSF_STATUS_CODE)
    if(NOT TSF_STATUS_CODE EQUAL 0)
        message(WARNING "TinySoundFont download failed — SF2 unit disabled")
    endif()
endif()
set(TSF_INCLUDE_DIR "${CMAKE_BINARY_DIR}/tsf_include")

# tinyfiledialogs (native OS file picker dialog)
set(TFD_DIR "${CMAKE_BINARY_DIR}/tfd_include")
if(NOT EXISTS "${TFD_DIR}/tinyfiledialogs.c")
    file(MAKE_DIRECTORY "${TFD_DIR}")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/native-toolkit/libtinyfiledialogs/master/tinyfiledialogs.c"
        "${TFD_DIR}/tinyfiledialogs.c" STATUS TFD_STATUS_C)
    file(DOWNLOAD
        "https://raw.githubusercontent.com/native-toolkit/libtinyfiledialogs/master/tinyfiledialogs.h"
        "${TFD_DIR}/tinyfiledialogs.h" STATUS TFD_STATUS_H)
endif()

set(SOURCES
    src/main.c
    src/tracker.c
    src/input.c
    src/clap_host.c
    src/audio.c
    src/ui.c
    src/screen_song.c
    src/screen_pattern.c
    src/screen_instrument.c
    src/screen_menu.c
    src/units/osc.c
    src/units/drum.c
    src/units/sf2.c
    src/units/clap_unit.c
    src/units/delay.c
    src/units/dist.c
    src/units/reverb.c
    src/units/chorus.c
    src/units/flanger.c
    src/units/phaser.c
    src/units/gran.c
    src/units/unit_registry.c
)

# tinyfiledialogs — desktop only (no native dialogs in browser)
if(NOT EMSCRIPTEN)
    list(APPEND SOURCES ${TFD_DIR}/tinyfiledialogs.c)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${raylib_SOURCE_DIR}/src
    ${clap_SOURCE_DIR}/include
    ${TSF_INCLUDE_DIR}
    ${TFD_DIR}
    src
)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".mjs")
    target_link_options(${PROJECT_NAME} PRIVATE
        -sUSE_GLFW=3
        -sINITIAL_MEMORY=67108864
        -sALLOW_MEMORY_GROWTH=1
        -sFORCE_FILESYSTEM=1
        -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS']
        -sEXPORT_ES6=1
        -sMODULARIZE=1
        -sSINGLE_FILE=1
        -sSINGLE_FILE_BINARY_ENCODE=1
        -sNO_EXIT_RUNTIME=1
        -sASSERTIONS=0
        -sWASM=1
        --no-heap-copy
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE raylib m)
    if(APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    endif()
endif()
