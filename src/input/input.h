#ifndef GBS_OPUS_INPUT_H
#define GBS_OPUS_INPUT_H
#include "key.h"
#include <cstdint>

namespace gbs_opus
{
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

        static bool button_down();
        static bool mouse_pos();
    private:
        static class keyboard_state m_kbstate;
    };
}

#endif //GBS_OPUS_INPUT_H
