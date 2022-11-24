#ifndef GBS_OPUS_APP_UI_H
#define GBS_OPUS_APP_UI_H
#include "ui_window.h"

#include <imgui.h>
#include <image.h>

namespace gbs_opus
{

class control_ui : public ui::window {
public:
    explicit control_ui(gbs_opus::app *a);
    void init();
    ~control_ui();
private:
    void render() override;

    bool m_show_scope = true;
    bool m_show_registers = false;
    int m_ticksize = 4;

    image m_art;
};
}

#endif //GBS_OPUS_APP_UI_H
