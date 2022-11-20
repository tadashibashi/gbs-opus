#ifndef GBS_OPUS_APP_UI_H
#define GBS_OPUS_APP_UI_H
#include "ui_window.h"

#include <imgui.h>
class GPU_Image;

namespace gbs_opus
{
class menu_ui : public ui::mainmenu {
public:
    menu_ui() : ui::mainmenu("Main Menu") { }
private:
        void render() override;
        void show_menu_file();
};

class control_ui : public ui::window {
public:
    control_ui();
    void init();
    ~control_ui();
private:
    void render() override;

    bool m_show_scope = true;
    bool m_show_registers = false;
    int m_ticksize = 4;

    GPU_Image *m_art;
};
}

#endif //GBS_OPUS_APP_UI_H
