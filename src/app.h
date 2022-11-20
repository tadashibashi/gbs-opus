//
// Created by Aaron Ishibashi on 11/16/22.
//

#ifndef GBS_OPUS_APP_H
#define GBS_OPUS_APP_H
#include "ui/app_ui.h"

namespace gbs_opus
{
    class app {
    public:
        app() = default;
        ~app() = default;
        void run();

    private:
        void run_frame();
        void update();
        void draw();

        menu_ui m_menu;
        control_ui m_scope;
    };
}



#endif //GBS_OPUS_APP_H
