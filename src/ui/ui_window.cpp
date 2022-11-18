#include "ui_window.h"
#include <imgui.h>

namespace gbs_opus::ui
{
    void window::do_render()
    {
        ImGui::Begin(m_name.c_str(), &m_visible, m_flags);
        ImGui::SetWindowSize(m_size);
        render();
        ImGui::End();
    }

    void mainmenu::do_render()
    {
        if (m_visible && ImGui::BeginMainMenuBar())
        {
            render();
            ImGui::EndMainMenuBar();
        }
    }

}