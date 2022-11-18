#ifndef GBS_OPUS_AUDIO_ENGINE_H
#define GBS_OPUS_AUDIO_ENGINE_H
#include <string>

namespace gbs_opus::audio
{
    typedef void (*audio_callback_t)(void *data, short *out, int count);

    class engine {
    public:
        static bool init(long sample_rate, int buffer_size);
        static void close();

        [[nodiscard]] static long samplerate();
        [[nodiscard]] static int buffersize();
        static void update();
        static void set_pause(bool pause);
        static void load_gbs(const std::string &path, int track_num = 0);
    private:

        static void handler(void *data, short *out, int count);



    };
}

#endif //GBS_OPUS_AUDIO_ENGINE_H
