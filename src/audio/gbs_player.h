/// This class acts as the interface between the gbs_driver and the
/// user interface.
#ifndef GBS_OPUS_AUDIO_ENGINE_H
#define GBS_OPUS_AUDIO_ENGINE_H
#include <string>
#include <delegate.h>
#include "gbs/libgbs.hpp"

#include <audio/playlist.h>
#include "gbs/gbs_meta.h"
#include "plugout_driver.h"

#define MAXOCTAVE 9

struct output_plugin;

namespace gbs_opus
{
    struct displaytime {
        long played_min;
        long played_sec;
        long total_min;
        long total_sec;
    };


    // TODO: Make a normal non-static class. Use a global gbs_player var for simplification.
    // 1. Move all functions from player.c to this class
    class gbs_player {
        struct impl;
    public:
        gbs_player();
        ~gbs_player();
        gbs_player(gbs_player &) = delete;
        gbs_player &operator=(gbs_player &) = delete;

    public:
        bool init(long sample_rate, int16_t buffer_size, plugout_type p = plugout_type::SDL);
        bool init(plugout_type p = plugout_type::SDL);
        void close();

        void update();

        [[nodiscard]] bool is_paused() const;
        void set_pause(bool pause);
        void toggle_pause();
        bool load(const std::string &path);
        [[nodiscard]] bool is_running() const;
        bool play_track(int track);
        [[nodiscard]] const displaytime &current_displaytime() const;
        bool play_prev();
        bool play_next();


        // Pauses the song if it's not already, and plays ticks
        void play_ticks(int count);
        [[nodiscard]] uint8_t io_peek(uint32_t addr) const;
        [[nodiscard]] uint8_t *io_hack();

        void toggle_mute(int chan);

        // Gets the mute status on the selected channel.
        // chan must be between 0 and 3 inclusviely.
        // Returns -1 on gbs not loaded, 1 on true, 0 on false.
        [[nodiscard]] long get_mute(int chan) const;

        [[nodiscard]] int current_track() const;

        /// Returns the total tracks in the playlist
        [[nodiscard]] size_t total_tracks() const;

        // Gets whether gbs is loaded in the player or not.
        [[nodiscard]] bool is_loaded() const;

        /// Gets the gbs album metadata/tags
        [[nodiscard]] const gbs_meta *meta() const;

        [[nodiscard]] class gbs_driver *driver() const;

    public: // Callbacks

        [[nodiscard]] PlaylistMode playlist_mode();
    private:
        impl *m;
    };
}

#endif //GBS_OPUS_AUDIO_ENGINE_H
