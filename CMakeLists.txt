cmake_minimum_required(VERSION 3.23)
project(gbs_opus)

set(CMAKE_CXX_STANDARD 17)


# Submodules
add_subdirectory(lib)


# Program Binary
add_executable(gbs_opus src/main.cpp src/audio/plugout_app.cpp src/systems.h src/systems.cpp
        src/app.cpp src/app.h src/audio/engine.cpp src/audio/engine.h
        src/input/input.cpp src/input/input.h src/input/key.h src/ui/ui_window.h
        src/ui/app_ui.h src/ui/ui_window.cpp src/ui/app_ui.cpp
        src/input/keyboard_state.cpp src/input/keyboard_state.h
        src/systems/system.h src/systems/system_mgr.cpp src/systems/system_mgr.h
        src/ui/actions.cpp src/ui/actions.h src/gb_helper.h

        lib/implot/implot.cpp lib/implot/implot_items.cpp
        )


# Platform Libraries
if (APPLE)
    set(NATIVE_DIALOG_LIBS "-framework AppKit" nfd)
    set(PLATFORM_LIBS ${NATIVE_DIALOG_LIBS})

elseif(WIN32)
    set(NATIVE_DIALOG_LIBS comctl32 nfd)
    set(PLATFORM_LIBS SDL2main ${NATIVE_DIALOG_LIBS})

endif()

target_link_directories(gbs_opus PRIVATE lib/nativefiledialog/build/lib/Release/x64)
target_link_libraries(gbs_opus PRIVATE SDL2 ${PLATFORM_LIBS} imgui gbsplay z)
target_include_directories(gbs_opus PRIVATE
        lib/SDL/include lib/nativefiledialog/src/include src lib/gbsplay
        lib/implot)

# MacOS Bundle Info
#if (APPLE)
#    set_target_properties(gbs_opus PROPERTIES
#            MACOSX_BUNDLE TRUE
#            MACOSX_FRAMEWORK_IDENTIFIER com.aaronishibashi.gbs-opus
#            RESOURCE "${RESOURCE_FILES}"
#            )
#endif()