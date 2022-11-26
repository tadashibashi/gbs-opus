#ifndef GBS_OPUS_PLUGIN_H
#define GBS_OPUS_PLUGIN_H

namespace gbs_opus
{
    class plugin {
    public:
        plugin();
        ~plugin();



        void load();
        void close();

        // Called when the plugin opens, fires opus.init()
        void script_init();

        // Called when the plugin closes, fires opus.close()
        void script_close();

        // Called every GB step, fires lua opus.update(dt)
        void script_update();

        // Draw stuff here, fires lua opus.draw()
        void script_draw();
    private:
    };
}

#endif //GBS_OPUS_PLUGIN_H
