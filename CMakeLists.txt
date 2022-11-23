cmake_minimum_required(VERSION 3.23)
project(gbs_opus)

set(CMAKE_CXX_STANDARD 17)

if (APPLE)
    SET(MACOSX_DEPLOYMENT_TARGET ${DARWIN_MAJOR_SDK_VERSION}.${DARWIN_MINOR_SDK_VERSION})
    SET(CMAKE_CXX_FLAGS "--sysroot ${CMAKE_OSX_SYSROOT} ${CMAKE_CXX_FLAGS}")
    MESSAGE("Setting MACOSX_DEPLOYMENT_TARGET to '${MACOSX_DEPLOYMENT_TARGET}'.")
endif()


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
        src/gbs/meta_io.h src/gbs/libgbs.hpp src/mathf.cpp src/mathf.h src/gbs/gbs_driver.cpp src/gbs/gbs_driver.h src/gbs/impl/gbs_driver_impl.cpp src/gbs/impl/gbs_driver_impl.h)


# Platform Libraries
if (APPLE)
    set(NATIVE_DIALOG_LIBS "-framework AppKit" nfd z "-framework OpenGL")
    set(PLATFORM_LIBS ${NATIVE_DIALOG_LIBS})

elseif(WIN32)
    set(NATIVE_DIALOG_LIBS comctl32 nfd zlib opengl32)
    set(PLATFORM_LIBS SDL2main ${NATIVE_DIALOG_LIBS})

endif()

target_link_directories(gbs_opus PRIVATE lib/nativefiledialog/build/lib/Release/x64)
target_link_libraries(gbs_opus PRIVATE SDL2 ${PLATFORM_LIBS} imgui gbsplay SDL_gpu)
target_include_directories(gbs_opus PRIVATE
        lib/SDL/include lib/nativefiledialog/src/include src lib/gbsplay
        lib/implot lib/json lib/sdl-gpu/include lib/json/include)

# MacOS Bundle Info
#if (APPLE)
#    set_target_properties(gbs_opus PROPERTIES
#            MACOSX_BUNDLE TRUE
#            MACOSX_FRAMEWORK_IDENTIFIER com.aaronishibashi.gbs-opus
#            RESOURCE "${RESOURCE_FILES}"
#            )
#endif()