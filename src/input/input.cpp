#include "input.h"
#include "keyboard_state.h"

#include <SDL.h>
#include <imgui_impl_sdl.h>

#include <filesystem>

namespace gbs_opus
{

    keyboard_state input::m_kbstate;
    uint32_t input::m_mstate, input::m_mstate_last;
    vec2 input::m_mpos, input::m_mpos_last;

    delegate<void(const SDL_DropEvent &ev)> input::on_dropfile;
    delegate<void(const SDL_WindowEvent &ev)> input::on_windowclose;
    delegate<void(const SDL_QuitEvent &ev)> input::on_quit;

    void input::init()
    {
        m_kbstate.init();
    }

    void input::close()
    {
        m_kbstate.close();
    }

    void input::process()
    {
        m_kbstate.process_input();

        m_mstate_last = m_mstate;
        m_mpos_last = m_mpos;

        SDL_Event ev;
        while(SDL_PollEvent(&ev))
        {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT)
                on_quit.try_invoke(ev.quit);
            if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE)
                on_windowclose.try_invoke(ev.window);
            if (ev.type == SDL_DROPFILE)
                on_dropfile.invoke(ev.drop);
        }

        int x, y;
        m_mstate = SDL_GetMouseState(&x, &y);
        m_mpos = {(float)x, (float)y};
    }

    bool input::key_down(key k) {
        return m_kbstate.isdown(k);
    }

    bool input::key_up(key k) {
        return m_kbstate.isup(k);
    }

    bool input::key_pressed(key k) {
        return m_kbstate.justdown(k);
    }

    bool input::key_released(key k) {
        return m_kbstate.justup(k);
    }

    const vec2 &input::mouse_pos() {
        return m_mpos;
    }

    const vec2 &input::mouse_lastpos() {
        return m_mpos_last;
    }

    bool input::mouse_moved() {
        return m_mpos.x != m_mpos_last.x || m_mpos.y != m_mpos_last.y;
    }

    bool input::button_down(mbutton b) {
        return m_mstate & SDL_BUTTON((int)b);
    }

    bool input::button_up(mbutton b) {
        return !(m_mstate & SDL_BUTTON((int)b));
    }

    bool input::button_pressed(mbutton b) {
        auto mask = SDL_BUTTON((int)b);
        return (m_mstate & mask) && !(m_mstate_last & mask);
    }

    bool input::button_released(mbutton b) {
        auto mask = SDL_BUTTON((int)b);
        return !(m_mstate & mask) && (m_mstate_last & mask);
    }

}

