# gbs_opus dependencies

# ===== SDL-related =================================================================
# For sdl-gpu's find SDL2
set(SDL2_INCLUDE_DIR SDL/include)
set(SDL2_LIBRARY SDL2)
set(SDL2MAIN_LIBRARY SDL2main)

if (NOT EMSCRIPTEN)
    # Emscripten uses official ports of these libraries, no need to include
    add_subdirectory(SDL)
    set(SDL2TTF_BUILD_SHARED_LIBS FALSE)
    set(SDL2TTF_VENDORED TRUE) # use included harfbuzz & freetype submodules
    set(BUILD_FRAMEWORK FALSE) # for MacOS, we don't need a framework
    add_subdirectory(SDL_ttf)
endif()



add_subdirectory(sdl-gpu)

if (EMSCRIPTEN)
    set(GPU_CompileDefs
            SDL_GPU_DISABLE_OPENGL # use GLES3 by disabling the following...
            SDL_GPU_DISABLE_GLES_2
            SDL_GPU_DISABLE_GLES_1
            MIN_WEBGL_VERSION=2    # use WebGL 2
            MAX_WEBGL_VERSION=2)
    set_target_properties(SDL_gpu PROPERTIES
            COMPILE_DEFINITIONS "${GPU_CompileDefs}")

elseif(MSVC) # Unsupported currently, use MinGW
    # In MSVC, if <cmath> is included after an SDL header, duplicate M_PI defines
    # occur. Setting HAVE_M_PI informs SDL2 to not provide an extra M_PI for us.
    set_property(TARGET SDL2 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS HAVE_M_PI)

    # GBSPlay requires that endianness defines are set in the compiler
    include(TestBigEndian)
    TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
    if (IS_BIG_ENDIAN)
        set(ENDIAN 4321)
    else()
        set(ENDIAN 1234)
    endif()
    add_compile_definitions(__ORDER_BIG_ENDIAN__=4321 __BYTE_ORDER__=${ENDIAN} __ORDER_LITTLE_ENDIAN__=1234)
elseif(WIN32)
    target_link_libraries(SDL_gpu opengl32 glu32)

endif()

# ===== ImGUI =========================================================================
add_library(imgui
        imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_demo.cpp
        imgui/backends/imgui_impl_sdl.cpp imgui/backends/imgui_impl_opengl3.cpp)

target_link_libraries(imgui PRIVATE SDL2)
target_include_directories(imgui PUBLIC SDL/include ./imgui ./imgui/backends)

# ===== LibGBS ============================================================================

# Build libgbs' impulse.h and config.h files

if (NOT WIN32)
    add_custom_command(OUTPUT gbsplay/impulse.h gbsplay/config.h
            COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/gbsplay && make impulse.h
            COMMENT "Making gbsplay impulse.h and config.h")
else()
    add_custom_command(OUTPUT gbsplay/impulse.h gbsplay/config.h
            COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/gbsplay && mingw32-make impulse.h
            COMMENT "Making gbsplay impulse.h and config.h")
endif()

add_library(gbs
        gbsplay/gbcpu.c gbsplay/gbhw.c gbsplay/mapper.c gbsplay/gbs.c
        gbsplay/crc32.c gbsplay/util.c gbsplay/cfgparser.c
        gbsplay/plugout_wav.c gbsplay/plugout_vgm.c
        gbsplay/plugout_stdout.c gbsplay/plugout_iodumper.c
        gbsplay/plugout_altmidi.c
        gbsplay/plugout_midi.c gbsplay/midifile.c gbsplay/filewriter.c
        gbsplay/impulse.h gbsplay/config.h)

if (MACOS)
    target_link_libraries(gbs PRIVATE z)
elseif(WIN32)
    target_link_libraries(gbs PRIVATE z)
else()
    message(ERROR "Platform not supported yet")
endif()

target_include_directories(gbs PRIVATE SDL/include)

# ===== Native File Dialog =================================================================
if (MACOS)
    add_library(nfd nativefiledialog/src/nfd_cocoa.m nativefiledialog/src/nfd_common.c)
    target_include_directories(nfd PRIVATE
            nativefiledialog/src/include)
    target_link_libraries(nfd PUBLIC "-framework AppKit")
elseif(WIN32)
    add_library(nfd nativefiledialog/src/nfd_win.cpp nativefiledialog/src/nfd_common.c)
    target_include_directories(nfd PRIVATE
            nativefiledialog/src/include)
    target_link_libraries(nfd PUBLIC comctl32)
elseif(LINUX)

endif()
