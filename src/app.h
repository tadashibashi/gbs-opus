#ifndef GBS_OPUS_APP_H
#define GBS_OPUS_APP_H
union SDL_Event;

namespace gbs_opus
{
    class app {
    public:
        app();
        ~app() = default;
        void run();
        void draw();
        void run_frame();
        void update();
        class gbs_player *player() { return m_player; }
        [[nodiscard]] bool is_running() const { return m_running; }
    private:
    friend int filter_event(void *userdata, SDL_Event *event);


        // class menu_ui *m_menu;
        class control_ui *m_ctrl_ui;
        class gbs_player *m_player;

        bool m_running = false;
        bool m_resized = false;
    };
}



#endif //GBS_OPUS_APP_H
