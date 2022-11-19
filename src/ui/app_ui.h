#ifndef GBS_OPUS_APP_UI_H
#define GBS_OPUS_APP_UI_H
#include "ui_window.h"

#include <imgui.h>

namespace gbs_opus
{
class menu_ui : public ui::mainmenu {
public:
    menu_ui() : ui::mainmenu("Main Menu") { }
private:
        void render() override;
        void show_menu_file();
};

class scope_ui : public ui::window {
public:
    scope_ui();
    void init();
private:
    void render() override;

    bool m_show_scope = true;
    int m_ticksize = 4;
    std::string m_ticksize_text;
};
}

#endif //GBS_OPUS_APP_UI_H
