#include "app_ui.h"
#include "app.h"
#include "audio/gbs_player.h"
#include "gbs/gbs_driver.h"

#include "systems.h"

#include <iostream>
#include <vector>
#include "actions.h"
#include "imgui_memory_editor.h"
extern "C" {
#include <libgbs.h>
#include <gbhw.h>
}
#include <atomic>

#include "gbs/gb_helper.h"

#include "input/input.h"

#include <implot.h>

#include <filesystem>

#include <SDL_gpu.h>

#include "mathf.h"

long mute_channel[4];


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

    void menu_ui::show_menu_file()
    {
        if (ImGui::MenuItem("Open", "Ctrl+O"))
        {
//            if (is_running()) toggle_pause();
//            gbs_player::set_pause(true);
//            auto path = actions::open_file_dialog();
//            gbs_player::load_gbs(path);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit", "Ctrl+Q"))
        {
            systems::quit();
        }
    }



    std::vector<std::vector<float>> scope_display;

    static MemoryEditor memedit;
    extern float sdl_volume;

    static const int ScopeTrailSize = 1;
    static const int ScopeBufferDiv = 2;
    static std::atomic_int currentScopeIndex = 0;

    static const int ControlWindowWidth = 400;

    void control_ui::render()
    {
        ImGui::ShowDemoWindow();
        auto &player = *app()->player();

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    auto path = std::filesystem::path(actions::open_file_dialog());
                    player.load(path);

                    if (m_art)
                        GPU_FreeImage(m_art);
                    m_art = nullptr;

                    // Find .png, or .bmp
                    for (const auto& entry : std::filesystem::directory_iterator {path.parent_path()})
                    {
                        if (entry.path().extension() == ".png" || entry.path().extension() == ".bmp")
                        {
                            auto img = GPU_LoadImage(entry.path().c_str());
                            if (!img)
                            {
                                std::cerr << "Error: Failed to load image!\n";
                                continue;
                            }

                            m_art = img;
                            break;
                        }
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

        if (player.is_loaded())
        {
            auto meta = player.meta();

            title = meta->title().c_str();
            author = meta->composer().c_str();
            copyright = meta->copyright().c_str();

            auto &songdata = meta->tracks()[player.current_track()];

            song_title = songdata.title.c_str();
            subsong = player.current_track() + 1;

            time = player.current_displaytime();
            //progress = (time.played_min*60.f + time.played_sec) / (time.total_min*60.f + time.total_sec);
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
                const ImVec2 pmax{winpos.x + 12 + 160, winpos.y + 44 + 160 - winscroll};
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
                auto bufsize = player.driver()->buffer_size();
                const int ScopeSampleLength = (int)bufsize / ScopeBufferDiv;

                // Copy data into current line group
//                if (player.is_loaded())
//                {
//                    for (int i = 0; i < ScopeSampleLength; ++i)
//                    {
//                        scope_display[currentScopeIndex][i] =
//                                (float)player.buffer()[i];
//                    }
//                }



                // Display the Scope
                if (ImPlot::BeginPlot("Scope", {-1, 64}, ImPlotFlags_NoFrame | ImPlotFlags_CanvasOnly |
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
                        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 1.f-((float)i/(float)ScopeTrailSize));

                        ImPlot::PlotLine(("scope_"+std::to_string(i)).c_str(),
                                         scope_display[visiting].data(), scope_display[visiting].size(),
                                         1, 0, ImPlotLineFlags_Shaded, 0);
                        ImPlot::PopStyleVar();
                        visiting = wrap(visiting - 1, 0, ScopeTrailSize-1);
                    }
                    //auto *drawlist = ImPlot::GetPlotDrawList();
                    //drawlist->Flags |= ImDrawListFlags_AntiAliasedLines;
                    //currentScopeIndex = (currentScopeIndex + 1) % ScopeTrailSize;
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
                app()->player()->play_track(app()->player()->current_track());
            }
            ImGui::SameLine();
            if (!player.is_running())
            {
                if (ImGui::Button("Play", {36, 18}))
                {
                    player.toggle_pause();
                }
            }
            else
            {
                if (ImGui::Button("||", {36, 18}))
                {
                    player.toggle_pause();
                }

            }

            ImGui::SameLine();
            if (ImGui::Button("|<<", {36, 18}))
            {
                player.play_prev();
            }

            ImGui::SameLine();
            if (ImGui::Button(">>|", {36, 18}))
            {
                player.play_next();
            }
            ImGui::PopStyleVar();

            ImGui::EndTable();
        }

        ImGui::SetCursorPosY(236.f);


#pragma region MoreInfo

        if (ImGui::TreeNode("more info"))
        {
            if(ImGui::BeginTabBar("##moreinfobar"))
            {
                if (ImGui::BeginTabItem("Album"))
                {
                    ImGui::Text("Album info");
                    ImGui::TextWrapped("Lorem ipsum dolor sit amet, "
                                       "consectetur adipiscing elit. Aenean volutpat velit ut luctus dapibus. "
                                       "Integer lorem libero, volutpat et posuere et, dapibus ut lorem. Pellentesque "
                                       "vehicula mauris sit amet fringilla cursus. Praesent maximus diam lacus, et "
                                       "gravida nisl blandit et. Pellentesque sed risus elementum, malesuada diam vel, "
                                       "tincidunt leo. Duis vitae eleifend erat, facilisis semper diam. Vivamus congue "
                                       "ante urna, in fermentum est lobortis eu. Sed auctor lacinia massa, at malesuada "
                                       "diam sagittis ac. Ut laoreet urna libero, eleifend pharetra nisi feugiat sed. Morbi "
                                       "vulputate nunc ac felis auctor, nec sollicitudin nunc blandit. Sed vulputate, "
                                       "nunc vel aliquam vestibulum, odio nunc convallis neque, id aliquet velit diam "
                                       "vel eros. Donec eget convallis nisi, sit amet varius justo. Nunc sed tortor id "
                                       "velit euismod semper eu non mi.");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Track"))
                {
                    // Idea: Pages where you can scroll for more info
                    ImGui::Text("Current track info");
                    ImGui::TextWrapped("Lorem ipsum dolor sit amet, "
                                       "consectetur adipiscing elit. Aenean volutpat velit ut luctus dapibus. "
                                       "Integer lorem libero, volutpat et posuere et, dapibus ut lorem. Pellentesque "
                                       "vehicula mauris sit amet fringilla cursus. Praesent maximus diam lacus, et "
                                       "gravida nisl blandit et. Pellentesque sed risus elementum, malesuada diam vel, "
                                       "tincidunt leo. Duis vitae eleifend erat, facilisis semper diam. Vivamus congue "
                                       "ante urna, in fermentum est lobortis eu. Sed auctor lacinia massa, at malesuada "
                                       "diam sagittis ac. Ut laoreet urna libero, eleifend pharetra nisi feugiat sed. Morbi "
                                       "vulputate nunc ac felis auctor, nec sollicitudin nunc blandit. Sed vulputate, "
                                       "nunc vel aliquam vestibulum, odio nunc convallis neque, id aliquet velit diam "
                                       "vel eros. Donec eget convallis nisi, sit amet varius justo. Nunc sed tortor id "
                                       "velit euismod semper eu non mi.");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Spec"))
                {
                    // Idea: Pages where you can scroll for more info
                    ImGui::Text("GBS Version: %d", player.meta()->gbs_version());
                    ImGui::TextWrapped("Lorem ipsum dolor sit amet");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::TreePop();
        }


#pragma endregion MoreInfo


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


        //ImGui::Separator();

        ImGui::SetCursorPosX(0);
        if (ImGui::BeginTable("Tracks", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersH |
            ImGuiTableFlags_NoBordersInBody, {-1, 300}))
        {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, 0x22FFFFFF);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 32);
            ImGui::TableSetupColumn("");

            if (player.is_loaded())
            {
                for (int i = 0; i < player.meta()->tracks().size(); ++i)
                {
                    auto selected = player.current_track() == i;
                    auto &io = ImGui::GetIO();
                    auto &track = player.meta()->tracks()[i];
                    auto winpos = ImGui::GetWindowPos();
                    auto winsize = ImGui::GetWindowSize();
                    auto cpos = ImGui::GetCursorPos();
                    auto clickPos = io.MouseClickedPos[0];
                    float scrolly = ImGui::GetScrollY();
                    if (io.MouseDoubleClicked[0] &&
                        clickPos.x - winpos.x > 0 &&
                        clickPos.x - winpos.x < winsize.x &&
                        clickPos.y - winpos.y + scrolly > cpos.y &&
                        clickPos.y - winpos.y + scrolly < cpos.y + 32)
                        player.play_track(i);

                    ImGui::TableNextRow(0, 32);
                    if (selected)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF222222);
                    else
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF0A0A0A);

                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Text, 0x77EEEEEE);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
                    if (selected)
                        ImGui::Text("  >");
                    else
                        ImGui::Text("  %-4d", i + 1);
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
                    ImGui::PushStyleColor(ImGuiCol_Text, 0xEEEEEEEE);
                    ImGui::Text("   %s", track.title.c_str());
                    ImGui::PopStyleColor();
                }
            }

            ImGui::EndTable();
        }









        // Copyright
        TextCentered(std::string{"© "} + copyright);


        // input response, move this elsewhere
        if (input::key_pressed(key::Space))
            player.toggle_pause();


//        if (gbs && input::key_down(key::LeftCommand) || input::key_down(key::RightCommand))
//        {
//            if (input::key_pressed(key::One))
//            {
//                gbs_toggle_mute(gbs, 0);
//                mute_channel[0] = !mute_channel[0];
//            }
//            if (input::key_pressed(key::Two))
//            {
//                gbs_toggle_mute(gbs, 1);
//                mute_channel[1] = !mute_channel[1];
//            }
//            if (input::key_pressed(key::Three))
//            {
//                gbs_toggle_mute(gbs, 2);
//                mute_channel[2] = !mute_channel[2];
//            }
//            if (input::key_pressed(key::Four))
//            {
//                gbs_toggle_mute(gbs, 3);
//                mute_channel[3] = !mute_channel[3];
//            }
//
//            if (input::key_pressed(key::Right))
//                play_next_subsong(gbs);
//            if (input::key_pressed(key::Left))
//            {
//                if (time.played_min * 60 + time.played_sec > 0)
//                    gbs_init(gbs, subsong - 1);
//                else
//                    play_prev_subsong(gbs);
//            }
//
//        }

        // === Mute Buttons ===
        ImGui::NewLine();
        ImGui::NewLine();
        for (int i = 0; i < 4; ++i)
        {
            ImGui::SameLine();
            if (ImGui::Checkbox(("##mute_button" + std::to_string(i)).c_str(), (bool *)&mute_channel[i]))
            {
                player.toggle_mute(i);
            }
        }

        ImGui::NewLine();

        if (!player.is_running())
        {
            ImGui::SameLine();
            if (ImGui::Button("tick"))
            {
                player.play_ticks(m_ticksize);
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



        if (player.is_loaded())
        {
            // Read each wave sample in ram
            float wave_data[32];
            for (int i = 0; i < 16; ++i)
            {
                auto s = player.io_peek(0xff30 + i);
                wave_data[i*2+1] = float(s & 0b00001111u);
                wave_data[i*2]   = float((s & 0b11110000u) >> 4u);
            }

            ImGui::PlotLines("", wave_data, 32, 0, "Ch3 Wavetable", 1, 15, {300, 80});



        }

        if (m_show_registers)
        {
            if (player.is_loaded())
                memedit.DrawWindow("GB Registers", (void *)&player.io_hack()[16], 0x30, 0xFF10);
            else
                memedit.DrawWindow("GB Registers", nullptr, 0, 0xFF10);
        }




    }

    control_ui::control_ui(gbs_opus::app *a) : ui::window(a, "GBS Opus", ImGuiWindowFlags_MenuBar)
    {
        set_constraint({400, 400}, {400, 2000});
    }

    void control_ui::init()
    {
        set_flags(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);
        // Initialize scope
        scope_display.reserve(ScopeTrailSize);
        for (int i = 0; i < ScopeTrailSize; ++i)
        {
            auto &lines = scope_display.emplace_back();
            auto bufsize = app()->player()->driver()->buffer_size();
            lines.reserve(bufsize / ScopeBufferDiv);
            lines.assign(bufsize / ScopeBufferDiv, 0);
        }
        static int buf_counter;
        static std::function<void(void*,size_t)> write_cb =
                [](void *buf, size_t count)
                {
                    if (buf_counter % 4 == 0)
                        for (int i = 0; i < scope_display[currentScopeIndex].size() && i < count/2; ++i)
                        {
                            scope_display[currentScopeIndex][i] = (float)((int16_t *)buf)[i];
                            currentScopeIndex = (currentScopeIndex + 1) % ScopeTrailSize;
                        }
                    buf_counter = (buf_counter + 1) % 4;
                };
        app()->player()->driver()->on_write.add_listener(&write_cb);

        auto &style = ImPlot::GetStyle();
        style.PlotBorderSize = 0;
        style.LineWeight = 1;
        static const ImU32 cols[] = {
               0xFF555555, 0xFF777777, 0xFFAAAAAA
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