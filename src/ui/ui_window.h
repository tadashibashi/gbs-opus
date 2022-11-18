#ifndef GBS_OPUS_UI_WINDOW_H
#define GBS_OPUS_UI_WINDOW_H
#include <cstdint>
#include <string>
#include <utility>
#include "imgui.h"

namespace gbs_opus::ui
{
    class element {
    public:
        explicit element(const std::string &name) : m_name(name) { }
        virtual void do_render() = 0;

        [[nodiscard]] bool is_visible() const { return m_visible; }
        void set_visible(bool visible) { m_visible = visible; }

        [[nodiscard]] const std::string &name() const { return m_name; }
    protected:
        virtual void render() = 0;
        bool m_visible = true;
        std::string m_name = "Window";
    };

    class window : element {
    public:
        explicit window(const std::string &name, int flags = 0) : element(name),
            m_flags(flags) { }
        void do_render() override;
        void set_size(ImVec2 size) { m_size = size; }
        [[nodiscard]] ImVec2 get_size() const { return m_size; }

        void set_flags(int flags) { m_flags = flags; }
        [[nodiscard]] int get_flags() const { return m_flags; }
    private:
        ImVec2 m_size;
        int m_flags;

    };

    class mainmenu : element {
    public:
        explicit mainmenu(const std::string &name) : element(name) { }
        void do_render() override;
    };
}

#endif //GBS_OPUS_UI_WINDOW_H
