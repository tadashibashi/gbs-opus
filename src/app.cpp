#include "app.h"
#include "systems.h"

#include <audio/gbs_player.h>
#include <gbs/gbs_driver.h>
#include <input/input.h>
#include <ui/app_ui.h>

#include <SDL_timer.h>

#include <iostream>
#include <mutex>

static std::mutex winmutex;

#include <sol/sol.hpp>
#include <SDL_gpu.h>
#include <graphics/graphics.h>

namespace gbs_opus
{
    static sol::state lua;
    static graphics g;
    static image target_image;
    static GPU_Target *target;

    static void update_callback(cycles_t, const gbs_channel_status *chan)
    {
        auto env = lua["env"].get_or_create<sol::table>();

        sol::function sol_fn = env["update"];
        std::function<void(float, float, float, float, float, float, float, float)> fn = sol_fn;
        if (fn)
            fn(NOTE(chan[0].div_tc), chan[0].vol, NOTE(chan[1].div_tc), chan[1].vol,
               NOTE(chan[2].div_tc), chan[2].vol, NOTE(chan[3].div_tc), chan[3].vol);
    }

    static void test_plugin()
    {
        auto basepath = SDL_GetBasePath();
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
        lua.script("opus = {}");
        std::cout << "package path is " << lua["package"]["path"].get<std::string>() << '\n';
        lua["package"]["path"] = std::string(basepath) + "res/?.lua";
        std::cout << "package path is " << lua["package"]["path"].get<std::string>() << '\n';
        lua["opus"]["basepath"] = basepath;
        int i = 100;

        lua.set_function("increase", [&i](){++i;});

        std::cout << "i is:     " << i << '\n';
        lua.script("increase()");
        std::cout << "i is now: " << i << '\n';
        lua.script("print(opus.basepath) print(...)");
        lua.script("require('plug-driver')");
        SDL_free(basepath);

        target_image.create(320, 288);
        target = GPU_GetTarget(target_image.img());

        g.target(target);
        g.clear_color({120, 140, 170, 255});
        g.border_color({ 100, 100, 100, 255});
        g.fill_color({50, 50, 50, 255});
        auto lua_env = lua["env"].get_or_create<sol::table>();
        auto lua_g = lua_env["graphics"].get_or_create<sol::table>();
        lua_g.set_function("clear", []() {g.clear();});
        lua_g.set_function("setFillColor", [](float red, float green, float blue, float alpha)
                                            { g.fill_color({uint8_t(red * 255.f), uint8_t(green * 255.f),
                                                            uint8_t(blue * 255.f), uint8_t(alpha * 255.f)}); });
        lua_g.set_function("setClearColor", [](float red, float green, float blue, float alpha)
        { g.clear_color({uint8_t(red * 255.f), uint8_t(green * 255.f),
                        uint8_t(blue * 255.f), uint8_t(alpha * 255.f)}); });
        lua_g.set_function("drawCircleFill",
                           [](float x, float y, float radius, bool bordered)
                           {g.draw_circle_filled(x, y, radius, bordered);});
        lua_g.set_function("drawLine", [](float x1, float y1, float x2, float y2){g.draw_line(x1, y1, x2, y2);});
        lua.script("chan={[1]={note=0, vol=0}, [2]={note=0, vol=0}, [3]={note=0, vol=0}, [4]={note=0, vol=0}}");
        lua.script("function env.update(n1, v1, n2, v2, n3, v3, n4, v4) chan[1].note = n1 "
                   "chan[1].vol = v1 chan[2].note = n2 chan[2].vol = v2 chan[3].note = n3 "
                   "chan[3].vol = v3 chan[4].note = n4 chan[4].vol = v4 end");
    }



    static void update_plugin()
    {
    }

    static void draw_plugin()
    {
        if (ImGui::Begin("plugin.lua"))
        {
            lua.script("env.graphics.setClearColor(0, 0, 0, 1)\n"
                       "env.graphics.clear()\n"
                       "env.graphics.setFillColor(1, 0, 0, 1)\n"
                       "env.graphics.drawCircleFill(30, 240 - chan[1].note * 2, chan[1].vol, false)\n"
                       "env.graphics.setFillColor(0, 0, 1, 1)\n"
                       "env.graphics.drawCircleFill(60, 240 - chan[2].note * 2, chan[2].vol, false)\n"
                       "env.graphics.setFillColor(0, 1, 0, 1)\n"
                       "env.graphics.drawCircleFill(90, 240 - chan[3].note * 2, chan[3].vol, false)\n"
                       "env.graphics.setFillColor(1, 1, 1, 1)\n"
                       "env.graphics.drawCircleFill(120, 240 - chan[4].note * 2, chan[4].vol, false)\n"
                       );
            g.present();
            ImGui::Image((ImTextureID)target_image.id(), {(float)target_image.width(), (float)target_image.height()});

        }
        ImGui::End();
    }

    int filter_event(void *userdata, SDL_Event *event)
    {
        if (event->type == SDL_WINDOWEVENT)
        {
            if (event->window.event == SDL_WINDOWEVENT_RESIZED)
            {
                auto a = static_cast<app *>(userdata);
                winmutex.lock();
                systems::start_frame();
                a->update();
                GPU_SetWindowResolution(event->window.data1, event->window.data2);
                draw_plugin();
                systems::clear(10, 10, 10);
                a->draw();
                systems::present();
                a->m_resized = true;
                winmutex.unlock();
                return 0;
            }
            else if (event->window.event == SDL_WINDOWEVENT_MOVED)
            {
                GPU_SetWindowResolution(GPU_GetContextTarget()->w, GPU_GetContextTarget()->h);
            }

        }

        return 1;
    }

    void app::run()
    {
        if (systems::init())
        {
            SDL_SetEventFilter(filter_event, this);
            static std::function<void(const SDL_QuitEvent &)> quitcb =
                    [this](const auto &ev){ m_running = false; };
            input::on_quit.add_listener(&quitcb);
            m_ctrl_ui = new control_ui{this};
            m_player = new gbs_player;

            m_player->init(44100, 1024);
            m_player->driver()->on_step.add_listener(update_callback);
            m_ctrl_ui->init();
            test_plugin();
            m_running = true;
            while (m_running )
                run_frame();

            m_player->close();
            delete m_player;
            delete m_ctrl_ui;
            systems::shutdown();
        }
    }



    // Frame "conductor"
    void app::run_frame()
    {
        // Input
        systems::process_input();

        if (m_resized)
        {
            m_resized = false;
            return;
        }

        // Update
        systems::start_frame();
        update();
        update_plugin();
        draw_plugin();
        m_player->update();

        // Rendering
        systems::clear(10, 10, 10); // todo: variable clear color
        draw();

        systems::present();

    }


    // Our app's runtime code goes here
    void app::update()
    {
        if (input::key_pressed(key::O) &&
            (input::key_down(key::LeftCommand) || input::key_down(key::RightCommand)))
        {
            std::cout << "Open! " << SDL_GetTicks() << '\n';
        }

        m_ctrl_ui->do_render();
    }


    // Our drawing code go here
    void app::draw()
    {

    }

    app::app() : m_ctrl_ui{}, m_player{} {

    }

    void app::quit() {
        m_running = false;
    }

}
