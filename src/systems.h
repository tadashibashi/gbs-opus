

#ifndef GBS_OPUS_SYSTEMS_H
#define GBS_OPUS_SYSTEMS_H
#include <cstdint>
#include <string>

class SDL_Window;
class SDL_Renderer;
union SDL_Event;

namespace gbs_opus
{

    // Global class to control app systems
    namespace systems {
        bool init();
        void shutdown();

        SDL_Window *window();
        SDL_Renderer *renderer();

        // Render clear with color. Renders IMGUI as well
        void clear(uint8_t r, uint8_t g, uint8_t b);

        // Presents the rendered images
        void present();

        // Call this at the start of every update frame
        void start_frame();

        void process_input();

        [[nodiscard]] bool was_init();

        bool should_quit();

        std::string open_file_dialogue();

        void quit();
    };
}



#endif //GBS_OPUS_GRAPHICS_H