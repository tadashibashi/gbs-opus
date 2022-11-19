#include "engine.h"
#include "input/input.h"
#include <SDL_audio.h>
#include <iostream>

extern "C" {
#include "libgbs.h"
#include "player.h"
#include "plugout.h"
#include "gbs_internal.h"
}

#define CFG_FILTER_OFF "off"
#define CFG_FILTER_DMG "dmg"
#define CFG_FILTER_CGB "cgb"

struct filter_map {
    const char *name;
    enum gbs_filter_type type;
};

static const struct filter_map FILTERS[] = {
        { CFG_FILTER_OFF, FILTER_OFF },
        { CFG_FILTER_DMG, FILTER_DMG },
        { CFG_FILTER_CGB, FILTER_CGB },
        { nullptr, static_cast<gbs_filter_type>(-1)},
};

static const char *sound_name = PLUGOUT_DEFAULT;
static const char *filter_type = CFG_FILTER_DMG;


timespec pause_wait_time;
const char *sound_description;
extern const struct output_plugin plugout_midi;
namespace gbs_opus
{
    static int  step_handler(cycles_t cycles, const struct gbs_channel_status chan[]);
    extern const struct output_plugin plugout_app;

    int16_t audio::engine::s_buffer_size = 0;
    long audio::engine::s_sample_rate = 0;
    int16_t *audio::engine::s_buffer = nullptr;
    float audio::engine::s_speed = 1.f;

    gbs *gbs = nullptr;

    long audio::engine::sample_rate() { return s_sample_rate; }
    int audio::engine::buffer_size() { return s_buffer_size; }

    bool audio::engine::init(long sample_rate, int16_t buffer_size, plugout_type type)
    {
        // Reset the buffer
        if (s_buffer)
            delete[] s_buffer;

        s_buffer_size = buffer_size;
        s_buffer = new int16_t[s_buffer_size];
        s_sample_rate = sample_rate;

        // Load plugout
        switch(type)
        {
            case plugout_type::SDL:
                sound_open = plugout_app.open;
                sound_close = plugout_app.close;
                sound_write = plugout_app.write;
                sound_io = nullptr;
                sound_step = step_handler;
                sound_pause = nullptr;
                break;
            case plugout_type::MIDI:
                sound_name = plugout_midi.name;
                sound_description = plugout_midi.description;
                sound_open = plugout_midi.open;
                sound_close = plugout_midi.close;
                sound_write = plugout_midi.write;
                sound_io = plugout_midi.io;
                sound_step = plugout_midi.step;
                sound_pause = plugout_midi.pause;
            default:
                std::cerr << "Unknown or unsupported plugout type\n";
                break;
        }

        //load_plugout(type);

        return true;
    }

    int step_handler(cycles_t cycles, const struct gbs_channel_status chan[])
    {
        // This is where the note data should be drawn, perhaps put in another file.
        return 0;
    }

    static void stepcallback(struct gbs *gbs, cycles_t cycles, const struct gbs_channel_status chan[], void *priv)
    {
        sound_step(cycles, chan);
    }


    void audio::engine::load_gbs(const std::string &path, int track_num)
    {

        ::gbs *temp_gbs;
        //set_pause(true);
        temp_gbs = common_init2(path.c_str());
        if (!temp_gbs)
        {
            std::cerr << "Error loading gbs file!\n";
            return;
        }

        gbs_set_filter(temp_gbs, gbs_filter_type::FILTER_DMG);

        /* init additional callbacks */
        if (sound_step)
            gbs_set_step_callback(temp_gbs, stepcallback, nullptr);

        if (gbs)
        {
            gbs_close(gbs);
        }

        gbs = temp_gbs;
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

    const output_plugin *audio::engine::load_plugout(audio::plugout_type type)
    {
        const struct output_plugin *plugout;

        if (strcmp(sound_name, "list") == 0) {
            plugout_list_plugins();
            exit(0);
        }

        plugout = plugout_select_by_name(sound_name);
        if (plugout == nullptr) {
            fprintf(stderr, _("\"%s\" is not a known output plugin.\n\n"),
                    sound_name);
            exit(1);
        }

        sound_open = plugout->open;
        sound_skip = plugout->skip;
        sound_io = plugout->io;
        sound_step = plugout->step;
        sound_write = plugout->write;
        sound_close = plugout->close;
        sound_pause = plugout->pause;
        sound_description = plugout->description;

        if (plugout->flags & PLUGOUT_USES_STDOUT) {
            verbosity = 0;
        }

        return plugout;
    }
}