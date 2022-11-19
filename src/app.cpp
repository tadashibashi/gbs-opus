#include "app.h"
#include "systems.h"
#include "input/input.h"
#include "SDL_timer.h"
#include <iostream>
#include "audio/engine.h"

namespace gbs_opus
{

    void app::run() {
        if (systems::init())
        {
            m_scope.init();
            //audio::engine::load_gbs("/Users/aaron/Downloads/Pokemon Card GB 2 - GRdan Sanjou (EMU)/PokemonCardGB2.gbs");
            while (!systems::should_quit())
                run_frame();

            systems::shutdown();
        }
    }

    // Frame "conductor"
    void app::run_frame()
    {
        systems::process_input();
        systems::start_frame();
        update();
        systems::clear(10, 10, 10); // todo: variable clear color
        draw();
        systems::present();
        audio::engine::update();
    }


    // Our app's runtime code goes here
    void app::update()
    {
        if (input::key_pressed(key::O) &&
            (input::key_down(key::LeftCommand) || input::key_down(key::RightCommand)))
        {
            std::cout << "Open! " << SDL_GetTicks() << '\n';
        }

        m_menu.do_render();
        m_scope.do_render();
    }


    // Our drawing code go here
    void app::draw()
    {

    }

}
