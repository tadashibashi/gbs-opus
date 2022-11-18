#include "app_ui.h"
#include "systems.h"
#include <imgui.h>
#include <iostream>
#include "actions.h"
#include "imgui_memory_editor.h"
extern "C" {
#include <libgbs.h>
#include <player.h>
#include <gbhw.h>
}
#include "audio/engine.h"
#include "gb_helper.h"
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

    float scope_display[1024];
    static MemoryEditor memedit;
    void scope_ui::render()
    {
        ImGui::ShowDemoWindow();
        ImGui::PlotLines("", scope_display, 1024, 0, nullptr, FLT_MAX, FLT_MAX,
                         ImVec2(300, 80));


        const char *title = "", *author = "", *copyright = "", *song_title = "";
        double ch1_midi = 0;
        int subsong = 0;
        float progress = 0;
        displaytime time;
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
        ImGui::Text("Title      %s", title);
        ImGui::Text("Artist     %s", author);
        ImGui::Text("Copyright  %s", copyright);
        ImGui::Spacing();
        ImGui::Text("Track      #%d %s", subsong, song_title);
        ImGui::Text("%02ld:%02ld/%02ld:%02ld", time.played_min, time.played_sec,
                    time.total_min, time.total_sec);
        float last_progress = progress;
        if (ImGui::SliderFloat("##", &progress, 0, 1.f, "", ImGuiSliderFlags_None))
        {
            if (is_running()) toggle_pause();
            // User moved slider update position?
            float ticks = (progress - last_progress) * (time.total_min*60.f + time.total_sec) * 10;
                gbs_step(gbs, 2048);
            std::cout << "Moved slider: " << progress << '\n';
        }


        if (ImGui::Button("<-"))
        {
            if (gbs)
                play_prev_subsong(gbs);
        }
        ImGui::SameLine();
        if (!is_running())
        {
            if (ImGui::ArrowButton("Play", ImGuiDir_Right))
            {
                toggle_pause();
            }
        }
        else
        {
            if (ImGui::Button("||"))
            {
                toggle_pause();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("->"))
        {
            if (gbs)
                play_next_subsong(gbs);
        }

        if (!is_running())
        {
            ImGui::SameLine();
            if (ImGui::Button("tick"))
            {
                gbs_step(gbs, refresh_delay);
            }
        }




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