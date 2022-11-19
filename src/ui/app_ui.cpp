#include "app_ui.h"
#include "systems.h"
#include <imgui.h>
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
#include "SDL_audio.h"
#include "input/input.h"
#include <implot.h>

extern long mute_channel[4];
extern long refresh_delay;

namespace gbs_opus
{
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
    static MemoryEditor memedit;
    extern float sdl_volume;

    static const int ScopeTrailSize = 7;
    static const int ScopeBufferDiv = 3;
    static int currentScopeIndex = 0;

    void scope_ui::render()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
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

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Show Oscilloscope", nullptr, m_show_scope))
                {
                    m_show_scope = !m_show_scope;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }







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
            if (ImPlot::BeginPlot("Scope", {-1, 100}, ImPlotFlags_NoFrame | ImPlotFlags_CanvasOnly | ImPlotFlags_NoMenus |
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
            subsong = status->subsong + 1;
            ch1_midi = getnote(status->ch[0].div_tc);
            song_title = status->songtitle;

            update_displaytime(&time, status);
            progress = (time.played_min*60.f + time.played_sec) / (time.total_min*60.f + time.total_sec);
        }


        ImGui::Text("%s", title);
        ImGui::Text("by %s", author);
        ImGui::Text("Copyright © %s", copyright);
        ImGui::Separator();
        ImGui::Text("Track  %3d%20s%02ld:%02ld/%02ld:%02ld", subsong,"", time.played_min, time.played_sec,
                    time.total_min, time.total_sec);




//        if (ImGui::SliderFloat("##", &progress, 0, 1.f, "", ImGuiSliderFlags_None))
//        {
//            //if (is_running()) toggle_pause();
//            // User moved slider update position?
////            float ticks = (progress - last_progress) * (time.total_min*60.f + time.total_sec) * 1024;
////
////            std::cout << "Moved slider: " << progress << '\n';
//        }

        if (ImGui::SliderFloat("vol", &sdl_volume, 0, 1.f, "%.3f", ImGuiSliderFlags_None))
        {

        }

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
        ImGui::SetCursorPosX(112.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
        if (ImGui::Button("<", {18, 18}))
        {
            if (gbs)
                play_prev_subsong(gbs);
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
        if (ImGui::Button(">", {18, 18}))
        {
            if (gbs)
                play_next_subsong(gbs);
        }

        ImGui::PopStyleVar();


        // controls, move this elsewhere
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

            memedit.DrawWindow("GB Registers", (void *)&gbs_io_get(gbs)[16], 0x30, 0xFF10);
        }




    }

    scope_ui::scope_ui() : ui::window("GBS Opus", ImGuiWindowFlags_MenuBar)
    {

    }

    void scope_ui::init()
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

        m_ticksize_text = "4   ";
    }
}