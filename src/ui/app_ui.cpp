#include "app_ui.h"
#include "systems.h"

#include <iostream>
#include <vector>
#include "actions.h"
#include "imgui_memory_editor.h"
extern "C" {
#include <libgbs.h>
#include <player.h>
#include <gbhw.h>
}
#include "audio/engine.h"
#include "gb_helper.h"

#include "input/input.h"
#include "m3u.h"

#include <implot.h>

#include <filesystem>

#include <SDL_gpu.h>

extern long mute_channel[4];
extern long refresh_delay;

namespace gbs_opus
{
    static void TextCentered(std::string text, float percentage = .5f) {
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * percentage);
        ImGui::Text("%s", text.c_str());
    }

    void menu_ui::render()
    {
        if (ImGui::BeginMenu("File"))
        {

            show_menu_file();
            ImGui::EndMenu();
        }
    }

    extern gbs *gbs;

    void menu_ui::show_menu_file()
    {
        if (ImGui::MenuItem("Open", "Ctrl+O"))
        {
            if (is_running()) toggle_pause();
            audio::engine::set_pause(true);
            auto path = actions::open_file_dialog();
            audio::engine::load_gbs(path);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit", "Ctrl+Q"))
        {
            systems::quit();
        }
    }

    /// Helper to wrap a range of numbers (inclusive on both lower and upper bounds)
    static int wrap(int kX, int const kLowerBound, int const kUpperBound)
    {
        int range_size = kUpperBound - kLowerBound + 1;

        if (kX < kLowerBound)
            kX += range_size * ((kLowerBound - kX) / range_size + 1);

        return kLowerBound + (kX - kLowerBound) % range_size;
    }

    std::vector<std::vector<float>> scope_display;
    std::vector<m3u> m3us;
    static MemoryEditor memedit;
    extern float sdl_volume;

    static const int ScopeTrailSize = 7;
    static const int ScopeBufferDiv = 3;
    static int currentScopeIndex = 0;

    static const int ControlWindowWidth = 400;

    void control_ui::render()
    {

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    // TODO: MOVE this to engine / player object
                    if (is_running()) toggle_pause();
                    audio::engine::set_pause(true);
                    auto path = std::filesystem::path{actions::open_file_dialog()};
                    auto ext = path.extension();

                    if (ext == ".gbs")
                    {
                        audio::engine::load_gbs(path);

                        if (m_art)
                            GPU_FreeImage(m_art);
                        m_art = nullptr;

                        // Find .png, or .bmp
                        for (const auto& entry : std::filesystem::directory_iterator {path.parent_path()})
                        {
                            if (entry.path().extension() == ".png" || entry.path().extension() == ".bmp")
                            {
                                auto img = GPU_LoadImage("/Users/aaron/Documents/Game Music Library/Game Boy/Junichi Masuda/Pokemon Red (EMU)/Red_EN_boxart.png");
                                if (!img)
                                {
                                    std::cerr << "Error: Failed to load image!\n";
                                    continue;
                                }

                                m_art = img;
                                break;
                            }
                        }

                        // Find .m3u, get info, load into gbs subsong info
                        std::vector<gbs_subsong_info> info;
                        m3us.clear();
                        auto status = gbs_get_status(gbs);
                        info.assign(status->songs, {});
                        m3us.assign(status->songs, {});
                        for (const auto& entry : std::filesystem::directory_iterator {path.parent_path()})
                        {
                            if (entry.path().extension() == ".m3u")
                            {
                                auto index = entry.path().filename().string().find_first_of(' ');
                                if (index == std::string::npos || index == 0)
                                {
                                    std::cout << "Warning: filename " << entry.path() <<
                                        ", must begin with a number indicating track number. "
                                        "Skipping file.\n";
                                    continue;
                                }
                                std::string num_str = entry.path().filename().string().substr(0, index);
                                int track_num = std::stoi(num_str)-1;
                                if (track_num > status->songs)
                                {
                                    std::cout << "Warning: file " << entry.path() << " track number is out of range. "
                                                                                     "Skipping file.\n";
                                    continue;
                                }

                                m3u data;
                                if (!data.open(entry.path()))
                                {
                                    std::cout << "Warning: failed to parse " << entry.path() <<
                                    ". Probably invalid file formatting\n";
                                    continue;
                                }
                                m3us[track_num] = data;

                                if (data.gbs_track_num >= status->songs)
                                {
                                    std::cout << "Warning: gbs track num in " << entry.path() <<
                                    " is too high. Skipping set.\n";
                                    continue;
                                }

                                auto &current_info = info[data.gbs_track_num];
                                current_info.title = m3us[track_num].track_title.c_str();
                                current_info.len = m3us[track_num].track_time * 1024;
                                current_info.fadeout = m3us[track_num].fade_time * 1024;
                            }
                        }
                        gbs_set_subsong_info(gbs, info.data(), info.size());
                        if (!is_running())
                            toggle_pause();
                    }
                    else if (ext == ".m3u")
                    {
                        audio::engine::load_m3u(path);
                        if (!is_running())
                            toggle_pause();
                    }



                }

                ImGui::Separator();

                if (ImGui::MenuItem("Quit", "Ctrl+Q"))
                {
                    systems::quit();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Show Oscilloscope", nullptr, m_show_scope))
                {
                    m_show_scope = !m_show_scope;
                }
                if (ImGui::MenuItem("Show Sound Registers", nullptr, m_show_registers))
                {
                    m_show_registers = !m_show_registers;
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        const char *title = "Title", *author = "Composer", *copyright = "", *song_title = "";
        double ch1_midi = 0;
        int subsong = 0;
        float progress = 0;
        displaytime time {
                .played_min = 0,
                .played_sec = 0,
                .total_min = 0,
                .total_sec = 0
        };

        if (gbs)
        {
            const auto data = gbs_get_metadata(gbs);
            const auto status = gbs_get_status(gbs);
            title = data->title;
            author = data->author;
            copyright = data->copyright;
            song_title = status->songtitle;
            subsong = status->subsong + 1;
            ch1_midi = getnote(status->ch[0].div_tc);
            song_title = status->songtitle;

            update_displaytime(&time, status);
            progress = (time.played_min*60.f + time.played_sec) / (time.total_min*60.f + time.total_sec);
        }

        if (ImGui::BeginTable("##Header", 2, ImGuiTableFlags_NoBordersInBody))
        {

            ImGui::TableNextColumn();

            // ALBUM ART
            if (m_art)
            {
                auto texid = GPU_GetTextureHandle(m_art);
                auto winpos = ImGui::GetWindowPos();
                auto winscroll = ImGui::GetScrollY();
                const ImVec2 pmin{winpos.x + 12, winpos.y + 44 - winscroll};
                const ImVec2 pmax{winpos.x + 12 + 180, winpos.y + 44 + 180 - winscroll};
                ImVec2 uvmin{0,0}, uvmax{1,1};

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
                ImGui::GetWindowDrawList()->AddImageRounded((ImTextureID)texid, pmin, pmax, uvmin, uvmax, 0xFFFFFFFF, 10.f);
                ImGui::PopStyleVar();

            }

            ImGui::TableNextColumn();

            // TITLE INFO
            const int offset_x = 14;
            ImGui::SetCursorPos({ImGui::GetCursorPosX() + offset_x, ImGui::GetCursorPosY()});
            ImGui::Text("%s", title);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);
            ImGui::Text("by %s", author);

            // OSCILLOSCOPE
            if (m_show_scope)
            {
                const int ScopeSampleLength = audio::engine::s_buffer_size / ScopeBufferDiv;

                // Copy data into current line group
                if (gbs)
                {
                    for (int i = 0; i < ScopeSampleLength; ++i)
                    {
                        scope_display[currentScopeIndex][i] =
                                (float)audio::engine::s_buffer[i];
                    }
                }



                // Display the Scope
                if (ImPlot::BeginPlot("Scope", {-1, 64}, ImPlotFlags_NoFrame | ImPlotFlags_CanvasOnly | ImPlotFlags_NoMenus |
                                                          ImPlotFlags_NoLegend | ImPlotFlags_NoMenus | ImPlotFlags_NoTitle))
                {
                    ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoHighlight | ImPlotAxisFlags_NoDecorations);
                    ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoHighlight | ImPlotAxisFlags_NoDecorations);
                    ImPlot::SetupAxesLimits(0, ScopeSampleLength, SHRT_MIN / 2, SHRT_MAX / 2, ImPlotCond_Always);

                    ImPlot::PushColormap("grayscale");
                    ImPlot::SetupAxisFormat(ImAxis_X1, "");
                    ImPlot::SetupAxisFormat(ImAxis_Y1, "");

                    for (int i = 0, visiting = wrap(currentScopeIndex - 1, 0, ScopeTrailSize-1); i < ScopeTrailSize; ++i)
                    {
                        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 1.f - ((float)i/(float)ScopeTrailSize));

                        ImPlot::PlotLine(("scope_"+std::to_string(i)).c_str(),
                                         scope_display[visiting].data(), scope_display[visiting].size(),
                                         1, 0, ImPlotLineFlags_Shaded, 0);
                        ImPlot::PopStyleVar();
                        visiting = wrap(visiting - 1, 0, ScopeTrailSize-1);
                    }
                    auto *drawlist = ImPlot::GetPlotDrawList();
                    drawlist->Flags |= ImDrawListFlags_AntiAliasedLines;
                    currentScopeIndex = (currentScopeIndex + 1) % ScopeTrailSize;
                    ImPlot::PopColormap();

                    ImPlot::EndPlot();
                }

            }


            ImGui::SetCursorPos({ImGui::GetCursorPosX() + offset_x, ImGui::GetCursorPosY() - 4});

            // TRACK CONTROLS
            ImGui::Text("%s", song_title);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);
            ImGui::Text("Track  %3d", subsong);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);
            ImGui::SameLine();
            ImGui::Text(" %02ld:%02ld/%02ld:%02ld", time.played_min, time.played_sec,
                        time.total_min, time.total_sec);

            ImGui::SetCursorPos({ImGui::GetCursorPosX() + 8.f, ImGui::GetCursorPosY() + 4.f});
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
            if (ImGui::Button("|<", {36, 18}))
            {
                if (gbs)
                    gbs_init(gbs, subsong-1);
            }
            ImGui::SameLine();
            if (!is_running())
            {
                if (ImGui::Button("Play", {36, 18}))
                {
                    toggle_pause();
                }
            }
            else
            {
                if (ImGui::Button("||", {36, 18}))
                {
                    toggle_pause();
                }

            }

            ImGui::SameLine();
            if (ImGui::Button("|<<", {36, 18}))
            {
                if (gbs)
                    play_prev_subsong(gbs);
            }

            ImGui::SameLine();
            if (ImGui::Button(">>|", {36, 18}))
            {
                if (gbs)
                    play_next_subsong(gbs);
            }
            ImGui::PopStyleVar();

            ImGui::EndTable();
        }

        ImGui::SetCursorPosY(232.f);





//        if (ImGui::SliderFloat("##", &progress, 0, 1.f, "", ImGuiSliderFlags_None))
//        {
//            //if (is_running()) toggle_pause();
//            // User moved slider update position?
////            float ticks = (progress - last_progress) * (time.total_min*60.f + time.total_sec) * 1024;
////
////            std::cout << "Moved slider: " << progress << '\n';
//        }

//        if (ImGui::SliderFloat("vol", &sdl_volume, 0, 1.f, "%.3f", ImGuiSliderFlags_None))
//        {
//
//        }

//        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, -1.f});
//        if (ImGui::SliderFloat("speed2", &audio::engine::s_speed, .250, 6.f, "%.3f",
//                            ImGuiSliderFlags_Logarithmic))
//        {
//            if (audio::engine::s_speed < .4f)
//            {
//                SDL_PauseAudio(true);
//            }
//            else
//            {
//                SDL_PauseAudio(false);
//            }
//
//
//            gbs_set_speed(gbs, audio::engine::s_speed);
//        }
//        ImGui::PopStyleVar();

        // Draw the play controls
        //ImGui::SetCursorPosX(112.f);


        ImGui::Separator();



        // Copyright
        TextCentered(std::string{"© "} + copyright);


        // input response, move this elsewhere
        if (input::key_pressed(key::Space))
            toggle_pause();


        if (gbs && input::key_down(key::LeftCommand) || input::key_down(key::RightCommand))
        {
            if (input::key_pressed(key::One))
            {
                gbs_toggle_mute(gbs, 0);
                mute_channel[0] = !mute_channel[0];
            }
            if (input::key_pressed(key::Two))
            {
                gbs_toggle_mute(gbs, 1);
                mute_channel[1] = !mute_channel[1];
            }
            if (input::key_pressed(key::Three))
            {
                gbs_toggle_mute(gbs, 2);
                mute_channel[2] = !mute_channel[2];
            }
            if (input::key_pressed(key::Four))
            {
                gbs_toggle_mute(gbs, 3);
                mute_channel[3] = !mute_channel[3];
            }

            if (input::key_pressed(key::Right))
                play_next_subsong(gbs);
            if (input::key_pressed(key::Left))
            {
                if (time.played_min * 60 + time.played_sec > 0)
                    gbs_init(gbs, subsong - 1);
                else
                    play_prev_subsong(gbs);
            }

        }

        // === Mute Buttons ===
        ImGui::NewLine();
        ImGui::NewLine();
        for (int i = 0; i < 4; ++i)
        {
            ImGui::SameLine();
            if (ImGui::Checkbox(("##mute_button" + std::to_string(i)).c_str(), (bool *)&mute_channel[i]))
            {
                gbs_toggle_mute(gbs, i);
            }
        }

        ImGui::NewLine();

        if (!is_running())
        {
            ImGui::SameLine();
            if (ImGui::Button("tick"))
            {
                if (gbs)
                    gbs_step(gbs, (long)(16.7 * m_ticksize));
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(72.f);
            if (ImGui::InputInt("##Tick size", &m_ticksize, 1, 100,
                                ImGuiInputTextFlags_CharsDecimal |
                                ImGuiInputTextFlags_CharsNoBlank))
            {
                if (m_ticksize < 1) m_ticksize = 1;
            }
        }



        if (gbs)
        {
            // Read each wave sample in ram
            float wave_data[32];
            for (int i = 0; i < 16; ++i)
            {
                auto s = gbs_io_peek(gbs, 0xff30 + i);
                wave_data[i*2+1] = float(s & 0b00001111u);
                wave_data[i*2]   = float((s & 0b11110000u) >> 4u);
            }

            ImGui::PlotLines("", wave_data, 32, 0, "Ch3 Wavetable", 1, 15, {300, 80});



        }

        if (m_show_registers)
        {
            if (gbs)
                memedit.DrawWindow("GB Registers", (void *)&gbs_io_get(gbs)[16], 0x30, 0xFF10);
            else
                memedit.DrawWindow("GB Registers", nullptr, 0, 0xFF10);
        }




    }

    control_ui::control_ui() : ui::window("GBS Opus", ImGuiWindowFlags_MenuBar)
    {
        set_constraint({400, 400}, {400, 2000});
    }

    void control_ui::init()
    {
        // Initialize scope
        scope_display.reserve(ScopeTrailSize);
        for (int i = 0; i < ScopeTrailSize; ++i)
        {
            auto &lines = scope_display.emplace_back();
            lines.reserve(audio::engine::s_buffer_size/ScopeBufferDiv);
            lines.assign(audio::engine::s_buffer_size/ScopeBufferDiv, 0);
        }

        auto &style = ImPlot::GetStyle();
        style.PlotBorderSize = 0;
        style.LineWeight = 2;
        static const ImU32 cols[] = {
                0xFF0A0A0A, 0xFF111111, 0xFF222222,
                0xFF333333, 0xFF444444, 0xFF555555,
                0xFFFFFFFF
        };

        ImPlot::AddColormap("grayscale", cols, sizeof(cols)/sizeof(ImU32));





    }

    control_ui::~control_ui()
    {
        if (m_art)
        {
            GPU_FreeImage(m_art);
        }
    }
}