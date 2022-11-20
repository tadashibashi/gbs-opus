#include "ui_window.h"
#include <imgui.h>

namespace gbs_opus::ui
{
    void window::do_render()
    {
        ImGui::SetNextWindowSizeConstraints(m_size_min, m_size_max);
        ImGui::Begin(m_name.c_str(), &m_visible, m_flags);

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