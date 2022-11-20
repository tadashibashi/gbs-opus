#ifndef GBS_OPUS_AUDIO_ENGINE_H
#define GBS_OPUS_AUDIO_ENGINE_H
#include <string>

struct output_plugin;

namespace gbs_opus::audio
{
    enum class plugout_type {
        SDL,
        GB,
        MIDI,
    };

    class engine {
    public:
        static bool init(long sample_rate, int16_t buffer_size, plugout_type = plugout_type::SDL);
        static void close();

        [[nodiscard]] static long sample_rate();
        [[nodiscard]] static int buffer_size();

        static void update();


        static void set_pause(bool pause);
        static void load_gbs(const std::string &path, int track_num = 0);
        static bool load_m3u(const std::string &path);

        static int16_t *s_buffer;
        static int16_t s_buffer_size;
        static long s_sample_rate;
        static float s_speed;
    private:
        static const output_plugin *load_plugout(plugout_type type);




    };
}

#endif //GBS_OPUS_AUDIO_ENGINE_H
