#include "engine.h"
#include "input/input.h"
#include <SDL.h>
extern "C" {
#include "libgbs.h"
#include "player.h"
}
#include <SDL_audio.h>
#include <iostream>

namespace gbs_opus
{
    const int ScopeLength = 1024;
    extern float scope_display[ScopeLength];
    static bool s_wasinit;
    static SDL_AudioSpec s_spec;
    static gbs_sound_cb cb_handle;
    gbs *gbs = nullptr;
    static struct gbs_output_buffer buf = {
            .data = NULL,
            .bytes = 8192,
            .pos  = 0,
    };
    timespec pause_wait_time;




    static int device;
    static SDL_AudioSpec obtained;
    #define PLAYBACK_MODE      0
    #define NO_CHANGES_ALLOWED 0
    #define UNPAUSE            0

    #ifdef SDL_AUDIO_ALLOW_SAMPLES_CHANGE
    #define SDL_FLAGS SDL_AUDIO_ALLOW_SAMPLES_CHANGE
    #else
    #define SDL_FLAGS NO_CHANGES_ALLOWED
    #endif
    static long sdl_open(enum plugout_endian *endian, long rate, long *buffer_bytes)
    {
        SDL_AudioSpec desired;
        if (SDL_WasInit(SDL_INIT_AUDIO) != SDL_INIT_AUDIO &&
            SDL_Init(SDL_INIT_AUDIO) != 0) {
            fprintf(stderr, _("Could not init SDL: %s\n"), SDL_GetError());
            return -1;
        }

        SDL_zero(desired);
        desired.freq = rate;
        desired.channels = 2;
        desired.samples = 1024;
        desired.callback = nullptr;

        switch (*endian) {
            case PLUGOUT_ENDIAN_BIG:    desired.format = AUDIO_S16MSB; break;
            case PLUGOUT_ENDIAN_LITTLE: desired.format = AUDIO_S16LSB; break;
            default:                    desired.format = AUDIO_S16SYS; break;
        }

        device = SDL_OpenAudioDevice(NULL, PLAYBACK_MODE, &desired, &obtained, SDL_FLAGS);
        if (device == 0) {
            fprintf(stderr, _("Could not open SDL audio device: %s\n"), SDL_GetError());
            return -1;
        }

        SDL_PauseAudioDevice(device, UNPAUSE);

        *buffer_bytes = obtained.samples * 4;
        return 0;
    }

    static ssize_t sdl_write(const void *buf, size_t count)
    {
        int overqueued = SDL_GetQueuedAudioSize(device) - obtained.size;
        float delaynanos = (float)overqueued / 4.0 / obtained.freq * 1000000000.0;
        struct timespec interval = {.tv_sec = 0, .tv_nsec = (long)delaynanos};
        if (overqueued > 0) {
            nanosleep(&interval, NULL);
        }
        if (SDL_QueueAudio(device, buf, count) != 0) {
            fprintf(stderr, _("Could not write SDL audio data: %s\n"), SDL_GetError());
            return -1;
        }

        for (int i = 0; i < count/2 && i < ScopeLength; ++i)
        {
            scope_display[i] = static_cast<const int16_t *>(buf)[i];
        }

        return count;
    }

//    static void sdl_close()
//    {
//        SDL_Quit();
//    }


    long audio::engine::samplerate() { return s_spec.freq; }
    int audio::engine::buffersize() { return s_spec.samples; }

    bool audio::engine::init(long sample_rate, int buffer_size)
    {
        sound_open = sdl_open;
        sound_write = sdl_write;
//        if (s_wasinit)
//            close();
//        //cb_handle = handler;
//        static SDL_AudioSpec as;
//        as.freq = (int)sample_rate;
//        as.format = AUDIO_S16SYS;
//        as.channels = 2;
//        as.callback = sdl_callback;
//        as.samples = buffer_size;
//
//        SDL_AudioSpec recd;
//        if (SDL_OpenAudio(&as, &recd) < 0)
//        {
//            std::cout << "Couldn't open SDL Audio: " << SDL_GetError() << '\n';
//            return false;
//        }
//        else
//        {
//            std::cout << "Audio initialized\n";
//            if (as.freq != recd.freq)
//                std::cout << "Requested " << as.freq << "Hz, but initialized with " <<
//                    recd.freq << "Hz\n";
//            if (as.samples != recd.samples)
//                std::cout << "Requested buffer size of " << as.samples << " samples, but " <<
//                    "received size of " << recd.samples << " samples.\n";
//        }
//
//        s_spec = recd;
//        s_wasinit = true;
        return true;
    }

    static void stepcallback(struct gbs *gbs, cycles_t cycles, const struct gbs_channel_status chan[], void *priv)
    {
        sound_step(cycles, chan);
    }


    void audio::engine::load_gbs(const std::string &path, int track_num)
    {
        if (gbs)
        {
            common_cleanup(gbs);
            gbs = nullptr;
        }

        //set_pause(true);
        gbs = common_init2(path.c_str());
        /* init additional callbacks */
        if (sound_step)
            gbs_set_step_callback(gbs, stepcallback, NULL);
//        gbs = gbs_open(path.c_str());
//        if (!gbs)
//        {
//            std::cerr << "Failed to load gbs\n";
//            return;
//        }
//
//        gbs_configure(gbs, -1, 60 * 2, 2, 2, 3);
//        gbs_configure_output(gbs, &buf, 48000);
//        pause_wait_time.tv_nsec = 33 * 1000000;
//        gbs_set_sound_callback(gbs, gbs_sound_handler, gbs);
//
//        gbs_init(gbs, 0);
        // TODO: move playing and track nums setting into another func, this is just to test.


        //set_pause(false);
    }

    void audio::engine::update()
    {
        if (gbs)
            step_emulation(gbs);
//        if (gbs)
//            gbs_step(gbs, 33);
//
//        nanosleep(&pause_wait_time, nullptr);
        if (input::key_pressed(key::A) && gbs)
            play_next_subsong(gbs);


    }

    void audio::engine::close()
    {
        if (gbs)
            common_cleanup(gbs);
//        if (gbs)
//            gbs_close(gbs);
        set_pause(true);
        SDL_CloseAudio();
    }

    void audio::engine::set_pause(bool pause)
    {
        if (pause)
        {
            SDL_PauseAudio(true);

            // be sure audio thread is not active
            SDL_LockAudio();
            SDL_UnlockAudio();
        }
        else
        {
            SDL_PauseAudio(false);
        }
    }
}