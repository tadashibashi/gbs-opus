#ifndef GBS_OPUS_UI_WINDOW_H
#define GBS_OPUS_UI_WINDOW_H
#include <cstdint>
#include <string>
#include <utility>
#include "imgui.h"

namespace gbs_opus
{
    class app;
}

namespace gbs_opus::ui
{
    class element {
    public:
        explicit element(gbs_opus::app *a, const std::string &name) :
                m_app{a}, m_name(name) { }
        virtual void do_render() = 0;

        [[nodiscard]] bool is_visible() const { return m_visible; }
        void set_visible(bool visible) { m_visible = visible; }

        [[nodiscard]] const std::string &name() const { return m_name; }
        gbs_opus::app *app() { return m_app; }
    protected:
        virtual void render() = 0;
        bool m_visible = true;
        std::string m_name = "Window";
        gbs_opus::app *m_app;
    };

    class window : public element {
    public:
        explicit window(gbs_opus::app *a, const std::string &name, int flags = 0) : element(a, name),
            m_flags(flags) { }
        void do_render() override;
        void set_size(ImVec2 size) { m_size = size; }
        void set_constraint(ImVec2 min, ImVec2 max) { m_size_min = min; m_size_max = max; }
        [[nodiscard]] ImVec2 get_size() const { return m_size; }

        void set_flags(int flags) { m_flags = flags; }
        [[nodiscard]] int get_flags() const { return m_flags; }
    private:
        ImVec2 m_size;
        ImVec2 m_size_min, m_size_max;
        int m_flags;

    };

    class mainmenu : public element {
    public:
        mainmenu(gbs_opus::app *a, const std::string &name) : element(a, name) { }
        void do_render() override;
    };
}

#endif //GBS_OPUS_UI_WINDOW_H
