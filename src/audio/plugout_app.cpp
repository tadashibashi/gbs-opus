#include <SDL.h>
#include <ctime>

extern "C" {
#include <plugout.h>
#include <common.h>
}


#include "gbs_player.h"

#define PLAYBACK_MODE      0
#define NO_CHANGES_ALLOWED 0
#define UNPAUSE            0

#ifdef SDL_AUDIO_ALLOW_SAMPLES_CHANGE
#define SDL_FLAGS SDL_AUDIO_ALLOW_SAMPLES_CHANGE
#else
#define SDL_FLAGS NO_CHANGES_ALLOWED
#endif

namespace gbs_opus
{
    static int device = 0;
    static SDL_AudioSpec obtained;

    float sdl_volume = .5f;

    static long sdl_open(plugout_endian *endian, long rate, long *buffer_bytes)
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

        if (device)
            SDL_CloseAudioDevice(device);

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
            nanosleep(&interval, nullptr);
        }

        SDL_MixAudioFormat((uint8_t *)buf, (uint8_t *)buf, AUDIO_S16SYS, count,
                           sdl_volume * SDL_MIX_MAXVOLUME * 2 - SDL_MIX_MAXVOLUME);

        if (SDL_QueueAudio(device, buf, count) != 0) {
            fprintf(stderr, _("Could not write SDL audio data: %s\n"), SDL_GetError());
            return -1;
        }


        return count;
    }

    static void sdl_pause(int pause)
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

    static void sdl_close()
    {
        SDL_AudioQuit();
    }

    extern const output_plugin plugout_app = {
            .name = "app",
            .description = "Uses SDL sound driver",
            .open = sdl_open,
            .write = sdl_write,
            .pause = sdl_pause,
            .close = sdl_close,
    };

}

