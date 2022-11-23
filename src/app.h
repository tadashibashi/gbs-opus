#ifndef GBS_OPUS_APP_H
#define GBS_OPUS_APP_H

namespace gbs_opus
{
    class app {
    public:
        app();
        ~app() = default;
        void run();

        class gbs_player *player() { return m_player; }
    private:
        void run_frame();
        void update();
        void draw();

        // class menu_ui *m_menu;
        class control_ui *m_ctrl_ui;
        class gbs_player *m_player;

        bool m_running = false;
    };
}



#endif //GBS_OPUS_APP_H
