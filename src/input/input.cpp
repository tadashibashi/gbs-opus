//
// Created by Aaron Ishibashi on 11/16/22.
//

#include "input.h"
#include "keyboard_state.h"

namespace gbs_opus
{

    keyboard_state input::m_kbstate;

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

}

