add_subdirectory(SDL)
add_subdirectory(game-music-emu)

add_library(imgui
        imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_demo.cpp
        imgui/backends/imgui_impl_sdl.cpp imgui/backends/imgui_impl_sdlrenderer.cpp)

target_link_libraries(imgui PRIVATE SDL2)
target_include_directories(imgui PUBLIC SDL/include ./imgui ./imgui/backends)

#add_library(gbs gbsplay/gbcpu.c  gbsplay/gbhw.c  gbsplay/mapper.c  gbsplay/gbs.c  gbsplay/crc32.c)
#        gbsplay/util.c gbsplay/plugout.c gbsplay/player.c gbsplay/cfgparser.c gbsplay/plugout_sdl.c)
#target_include_directories(gbs PUBLIC gbsplay)
#target_link_libraries(gbs PRIVATE z)