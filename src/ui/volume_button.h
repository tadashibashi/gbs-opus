#ifndef GBS_OPUS_VOLUME_BUTTON_H
#define GBS_OPUS_VOLUME_BUTTON_H
#include <ui/ui.h>
#include <image.h>
namespace gbs_opus
{

    class volume_button
    {
        const char * symbols[4] = {"x", ")", "))", ")))"};
        image m_vol_img;
    public:
        const float VolumeHoverTime = .5f;
        volume_button() : is_muted(), mouse_moved(), hover_counter() {}

        [[nodiscard]] bool is_popup_open() const
        {
            return this->hover_counter >= VolumeHoverTime;
        }

        void display(float *f)
        {
            float delta = ImGui::GetIO().DeltaTime;

            if (ImGui::BeginPopup("VolumePopup", ImGuiWindowFlags_NoMove))
            {
                if (!mouse_moved)
                {
                    if ((!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
                         ImGui::GetIO().MouseDelta != ImVec2{0, 0}) &&
                        !ImGui::GetIO().MouseDown[0])
                    {
                        mouse_moved = true;
                    }
                }
                else if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
                         !ImGui::GetIO().MouseDown[0])
                {
                    if (hover_counter <= 0)
                    {
                        ImGui::CloseCurrentPopup();
                        hover_counter = 0;
                    }

                    hover_counter -= delta * 2;
                }

                ImGui::VSliderFloat("##VolumeSlider", {8.f, 64.f}, f, 0, 1.f, "");

                ImGui::EndPopup();
            }


            if (!m_vol_img.is_loaded())
                m_vol_img.load("res/volume.png");

            if (ImGui::ImageButton((ImTextureID)m_vol_img.id(), {14.f, 14.f}, {0, 0},
                                   {1.f, 1.f}, -1, {0, 0, 0, 0}, {.75, .75, .75, 1}))
            {
                ImGui::OpenPopup("VolumePopup");
                this->hover_counter = VolumeHoverTime;
                mouse_moved = false;
            }



//            ImGui::SameLine();
//            ImGui::Text("vol");


            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && !is_muted)
            {
                if (hover_counter < VolumeHoverTime)
                {
                    hover_counter += delta;
                }
                else
                {
                    ImGui::OpenPopup("VolumePopup");
                    mouse_moved = false;
                }
            }

            const char *label;
            if (*f == 0) label = symbols[0];
            else if (*f < .4f) label = symbols[1];
            else if (*f < .8f) label = symbols[2];
            else label = symbols[3];
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 6);
            ImGui::Text("%-3s", label);
        }
    private:
        bool is_muted, mouse_moved;
        float hover_counter;
    };
}


#endif //GBS_OPUS_VOLUME_BUTTON_H
