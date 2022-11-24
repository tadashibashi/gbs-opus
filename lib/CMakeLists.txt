add_subdirectory(SDL)

set(SDL2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SDL/include)
set(SDL2_LIBRARY SDL2)

add_subdirectory(sdl-gpu)

add_library(imgui
        imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_demo.cpp
        imgui/backends/imgui_impl_sdl.cpp imgui/backends/imgui_impl_opengl3.cpp)

target_link_libraries(imgui PRIVATE SDL2)
target_include_directories(imgui PUBLIC SDL/include ./imgui ./imgui/backends)

add_custom_command(OUTPUT gbsplay/impulse.h gbsplay/config.h
        COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/gbsplay && make impulse.h
        COMMENT "Making gbsplay impulse.h and config.h")

add_library(libgbs
        gbsplay/gbcpu.c  gbsplay/gbhw.c  gbsplay/mapper.c  gbsplay/gbs.c  gbsplay/crc32.c
        gbsplay/util.c gbsplay/plugout.c gbsplay/cfgparser.c
        gbsplay/plugout_wav.c gbsplay/plugout_vgm.c gbsplay/plugout_stdout.c
        gbsplay/plugout_altmidi.c gbsplay/plugout_iodumper.c gbsplay/plugout_sdl.c
        gbsplay/plugout_midi.c gbsplay/midifile.c gbsplay/filewriter.c
        gbsplay/impulse.h gbsplay/config.h)


if (APPLE)
    target_link_libraries(libgbs PRIVATE SDL2 z)
elseif(WIN32)
    target_link_libraries(libgbs PRIVATE SDL2 zlib)
else()
    message(ERROR "Platform not supported yet")
endif()

add_library(nfd nativefiledialog/src/nfd_cocoa.m nativefiledialog/src/nfd_common.c)
target_include_directories(nfd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nativefiledialog/src/include)
