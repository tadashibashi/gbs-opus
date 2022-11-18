#ifndef GBS_OPUS_KEYBOARD_STATE_H
#define GBS_OPUS_KEYBOARD_STATE_H
#include <cstdint>
#include "key.h"

namespace gbs_opus
{
    class keyboard_state {
    public:
        keyboard_state();
        ~keyboard_state();

        void init();
        void close();

        void process_input() { copy_last_states(); }

        bool isdown(key k) const { return m_state[(int)k]; }
        bool isup(key k) const   { return !m_state[(int)k]; }
        bool justdown(key k) const { return m_state[(int)k] && !m_last[(int)k]; }
        bool justup(key k) const   { return !m_state[(int)k] && m_last[(int)k]; }

    private:
        void copy_last_states();
        const uint8_t *m_state;
        uint8_t       *m_last;
        int           m_size;
    };
}



#endif //GBS_OPUS_KEYBOARD_STATE_H
