#ifndef GBS_OPUS_PLUGIN_H
#define GBS_OPUS_PLUGIN_H
#include <SDL_gpu.h>
#include "graphics/graphics.h"

namespace gbs_opus
{
    class plugin {
    public:
        plugin();
        ~plugin();

        void load(const std::string &path);
        void close();

        // Called when the plugin opens, fires init()
        void script_init();

        // Called when the plugin closes, fires close()
        void script_close();

        // Called every GB step, fires lua update(dt)
        void script_update();

        // Draw stuff here, fires lua draw()
        void script_draw();
    private:
        GPU_Target *m_target;
        graphics m_graphics;
    };
}

#endif //GBS_OPUS_PLUGIN_H
