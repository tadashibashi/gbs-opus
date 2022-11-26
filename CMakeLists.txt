cmake_minimum_required(VERSION 3.16.5)

if ("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Darwin") # Detect MACOS
    set(MACOS TRUE)
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if (MACOS)
    # Universal binary
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
    project(gbs_opus C CXX OBJCXX OBJC)
    set(OBJCXX_STANDARD 17)
else()
    project(gbs_opus C CXX)
endif()


if (WIN32)
    # Set all targets to build into one directory. Paths are otherwise broken.
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} ) # for MSBuild generators
        string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
        set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR} )
        set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR} )
        set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR} )
    endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endif()

if (LINUX)
    set(OpenGL_GL_PREFERENCE "GLVND")
endif()

set(CMAKE_CXX_STANDARD 17)

# Submodules
add_subdirectory(lib)


# Program Binary
add_executable(gbs_opus src/main.cpp src/audio/plugout_app.cpp src/systems.h src/systems.cpp
        src/app.cpp src/app.h src/audio/gbs_player.cpp src/audio/gbs_player.h
        src/input/input.cpp src/input/input.h src/input/key.h src/ui/ui_window.h
        src/ui/app_ui.h src/ui/ui_window.cpp src/ui/app_ui.cpp
        src/input/keyboard_state.cpp src/input/keyboard_state.h
        src/systems/system.h src/systems/system_mgr.cpp src/systems/system_mgr.h
        src/ui/actions.cpp src/ui/actions.h src/gbs/gb_helper.h

        lib/implot/implot.cpp lib/implot/implot_items.cpp
        src/gbs/m3u.cpp src/gbs/m3u.h src/gbs/gbs_meta.cpp src/gbs/gbs_meta.h src/audio/plugout_driver.cpp
        src/audio/plugout_driver.h src/delegate.inl src/delegate.h
        src/audio/playlist.cpp src/audio/playlist.h src/gbs/meta_io.cpp
        src/gbs/meta_io.h src/gbs/libgbs.hpp src/mathf.cpp src/mathf.h src/gbs/gbs_driver.cpp src/gbs/gbs_driver.h
        src/gbs/impl/gbs_driver_impl.cpp src/gbs/impl/gbs_driver_impl.h src/ui/volume_button.h src/ui/ui.h
        src/image.cpp src/image.h src/plugin/plugin.cpp src/plugin/plugin.h src/plugin/lua.hpp src/plugin/lua_lib.c)


# Platform Libraries
if (MACOS)
    set(PLATFORM_LIBS nfd "-framework OpenGL" z)

elseif(WIN32)
    if (MSVC)
        message(ERROR "Only mingw64 compiler is supported on Windows.")
    endif()

    set(PLATFORM_LIBS SDL2main nfd opengl32 z mingw32)

elseif(LINUX)
    set(PLATFORM_LIBS nfd GL GLU z asound)
    #add_compile_options("pkg-config --cflags --libs gtk+-3.0")

    # Also, make sure to add gtk+3 via sudo apt-get apt-get libgtk-3-dev

endif()

target_link_directories(gbs_opus PRIVATE lib/nativefiledialog/build/lib/Release/x64)
target_link_libraries(gbs_opus PRIVATE SDL2 SDL2_ttf ${PLATFORM_LIBS} imgui gbs SDL_gpu)
target_include_directories(gbs_opus PRIVATE
        src
        lib/SDL_ttf
        lib/gbsplay
        lib/nfd-extended/src/include
        lib/SDL/include
        lib/implot
        lib/json
        lib/sdl-gpu/include
        lib/json/include
        lib/sol2/include
        lib/lua
        )


# Link Assets for testing. (Manually copy the folder on build)
include(cmake/link_assets.cmake)

Link_Assets(res)

# MacOS Bundle Info
#if (APPLE)
#    set_target_properties(gbs_opus PROPERTIES
#            MACOSX_BUNDLE TRUE
#            MACOSX_FRAMEWORK_IDENTIFIER com.aaronishibashi.gbs-opus
#            RESOURCE "${RESOURCE_FILES}"
#            )
#endif()

