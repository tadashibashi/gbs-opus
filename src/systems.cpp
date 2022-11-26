#include "systems.h"

#include "audio/gbs_player.h"
#include "input/input.h"
#include "ui/actions.h"


#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <implot.h>

#include <nfd.hpp>
#include <SDL.h>
#include <SDL_gpu.h>

#include <iostream>
#include <filesystem>

namespace gbs_opus
{
    // later: move input processing to input system, create listener system

    static GPU_Target *s_target;
    static void init_imgui();
    static void shutdown_imgui();

    bool systems::init()
    {
        // Setup window
        const auto window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        GPU_SetPreInitFlags(GPU_INIT_ENABLE_VSYNC);
        auto target = GPU_Init(1280, 720, window_flags);

        if (!target)
        {
            std::cerr << "Error initializaing SDL: " << SDL_GetError() << '\n';
            return false;
        }

        s_target = target;

        // Init other systems
        init_imgui();
        input::init();
        actions::init();
        NFD_Init();


        return true;
    }

    void systems::shutdown()
    {
        NFD_Quit();
        shutdown_imgui();
        input::close();
        if (s_target)
            GPU_FreeTarget(s_target);
        GPU_Quit();
    }

    void init_imgui()
    {
        if (!s_target || !s_target->context)
        {
            std::cerr << "Error: cannot initialize IMGUI without GPU target available\n";
            return;
        }
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO &io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsClassic();
        //ImGui::StyleColorsLight();

        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_PlotLines]              = ImVec4(0.63f, 0.63f, 0.63f, 1.00f);

        ImGuiStyle &style = ImGui::GetStyle();

        style.WindowPadding = {12.f, 9.f};
        style.FramePadding = {4.f, 2.f};
        style.CellPadding = {4.f, 2.f};
        style.ItemSpacing = {8.f, 8.f};
        style.ItemInnerSpacing = {4.f, 4.f};
        style.ScrollbarSize = 13.f;
        style.GrabMinSize = 5.f;

        style.WindowRounding = 8.0f;
        style.ChildRounding = 8.0f;
        //style.ScrollbarRounding = 7.f;

        style.WindowTitleAlign = {.5f, .4f};
        style.DisplaySafeAreaPadding = {5.f, 4.f};

        SDL_Window *window = SDL_GetWindowFromID(s_target->context->windowID);
        if (!window)
        {
            std::cerr << "Error: cannot initialize IMGUI, failed to get SDL window "
                         "from GPU target\n";
            return;
        }

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
        // GL 3.2 Core + GLSL 150
        const char* glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
        // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForOpenGL(window, s_target->context->context);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        io.Fonts->AddFontDefault();

        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);
    }


    static void shutdown_imgui()
    {
        ImPlot::DestroyContext();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void systems::clear(uint8_t r, uint8_t g, uint8_t b)
    {
        ImGui::Render();
        GPU_ClearColor(s_target, {r, g, b, 255u});
    }

    void systems::present()
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GPU_Flip(s_target);
    }

    void systems::start_frame()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void systems::process_input()
    {
        input::process();
    }

    std::string systems::open_file_dialogue() {
        nfdchar_t *outPath = nullptr;
        nfdfilteritem_t filterItem[1] = { "Gameboy Sound Files", "gbs,gbr,m3u,vgm,gb" };
        nfdresult_t result = NFD_OpenDialog( &outPath, filterItem, 1, nullptr);
        std::string ret;

        if ( result == NFD_OKAY ) {
            ret = std::string(outPath);
            free(outPath);
        }
        else if ( result == NFD_CANCEL ) {
            // Cancelled
        }
        else {
            std::cout << "Error: " << NFD_GetError() << '\n';
        }

        return ret;
    }
}
