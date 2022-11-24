#include "app.h"
#include "systems.h"
#include "input/input.h"
#include "SDL_timer.h"
#include <iostream>
#include "audio/gbs_player.h"

#include "ui/app_ui.h"

#include <mutex>

static std::mutex winmutex;

namespace gbs_opus
{
    int filter_event(void *userdata, SDL_Event *event)
    {
        if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)
        {
            auto a = static_cast<app *>(userdata);
            winmutex.lock();
            systems::start_frame();
            a->update();

            systems::clear(10, 10, 10);
            a->draw();
            systems::present();
            a->m_resized = true;
            winmutex.unlock();
            return 0;
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

            /// TODO: Get default audio with SDL_GetDefaultAudioInfo in an overload of this func.

            m_player->init(44100, 512);
            m_ctrl_ui->init();

            m_running = true;
            while (!systems::should_quit() && m_running )
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

}
