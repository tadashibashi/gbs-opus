#include "keyboard_state.h"
#include <SDL_keyboard.h>
#include <cstring>
#include <cstdlib>

namespace gbs_opus
{
    keyboard_state::keyboard_state() : m_last(), m_state(), m_size()
    {

    }

    keyboard_state::~keyboard_state()
    {
        close();
    }

    void keyboard_state::init()
    {
        m_state = SDL_GetKeyboardState(&m_size);
        m_last = (uint8_t *)malloc(m_size);
        copy_last_states();
    }

    void keyboard_state::copy_last_states()
    {
        memcpy(m_last, m_state, m_size);
    }

    void keyboard_state::close()
    {
        if (m_last)
        {
            free(m_last);
            m_last = nullptr;
        }
    }

}