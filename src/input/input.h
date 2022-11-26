#ifndef GBS_OPUS_INPUT_H
#define GBS_OPUS_INPUT_H
#include "key.h"
#include <delegate.h>
#include <cstdint>
#include <SDL_events.h>

namespace gbs_opus
{
    struct vec2 {
        float x;
        float y;
    };

    enum class mbutton {
        Left = 1,
        Middle,
        Right,
    };

    // simple static class for tracking keyboard and mouse input
    class input {
    public:

        static void init();
        static void close();

        // Called before polling input
        static void process();

        static bool key_down(key k);
        static bool key_up(key k);
        static bool key_pressed(key k);
        static bool key_released(key k);

        static bool button_down(mbutton b);
        static bool button_pressed(mbutton b);
        static bool button_up(mbutton b);
        static bool button_released(mbutton b);
        static const vec2 &mouse_pos();
        static bool mouse_moved();
        static const vec2 &mouse_lastpos();

        static delegate<void(const SDL_DropEvent &ev)> on_dropfile;
        static delegate<void(const SDL_WindowEvent &ev)> on_windowclose;
        static delegate<void(const SDL_WindowEvent &ev)> on_windowhide;
        static delegate<void(const SDL_QuitEvent &ev)> on_quit;

    private:
        static class keyboard_state m_kbstate;
        static uint32_t m_mstate, m_mstate_last;
        static vec2 m_mpos, m_mpos_last;
    };
}

#endif //GBS_OPUS_INPUT_H
