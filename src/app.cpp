#include "app.h"
#include "systems.h"
#include "input/input.h"
#include "SDL_timer.h"
#include <iostream>
#include "audio/gbs_player.h"

#include "ui/app_ui.h"

namespace gbs_opus
{

    void app::run() {
        if (systems::init())
        {
            static std::function<void(const SDL_QuitEvent &)> quitcb =
                    [this](const auto &ev){ m_running = false; };
            input::on_quit.add_listener(&quitcb);
            m_ctrl_ui = new control_ui{this};
            m_player = new gbs_player;


            m_ctrl_ui->init();
            m_player->init(48000, 512);

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
