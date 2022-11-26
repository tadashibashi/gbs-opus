/// Wraps a libgbs gbs object for easy management and access
#ifndef GBS_OPUS_GBS_DRIVER_H
#define GBS_OPUS_GBS_DRIVER_H
#include <cstdlib>
#include <audio/plugout_driver.h>
#include "delegate.h"


struct gbs;

namespace gbs_opus
{
    class gbs_driver {
        class impl;
    public:
        gbs_driver();
        ~gbs_driver();
        bool init(long sample_rate, int16_t buffer_size, plugout_type type);
        void close();

        void update();

        bool load(const char *filepath);

        bool play_song(int song_index);

        bool play_ticks(int ticks);

        uint8_t io_peek(uint16_t addr) const;

        uint8_t *io_get();

        void set_subsoundinfo(const class gbs_meta &meta);

        void set_paused(bool paused);

        [[nodiscard]] int gbs_version() const;
        [[nodiscard]] bool is_open() const;
        [[nodiscard]] bool is_paused() const;

        [[nodiscard]] int song_index() const;

        /// Gets time played in the current track in seconds
        [[nodiscard]] float tracktime_played() const;
        /// Gets total length of the current track in seconds
        [[nodiscard]] float tracktime_total() const;


        [[nodiscard]] bool is_mute(int chan) const;
        void set_mute(int chan, bool mute);

        [[nodiscard]] size_t song_count() const;
        [[nodiscard]] size_t buffer_size() const;
        [[nodiscard]] int16_t *buffer();

        [[nodiscard]] const char *filetype() const;

        /// Gets internal gbs object. Null if none loaded.
        struct gbs *gbs();

        [[nodiscard]] const plugout_driver &plugout() const;

    public: /// Callbacks
        // Callback for monitoring gb audio channel status
        delegate<void(cycles_t, const gbs_channel_status *chan)> on_step;

        // Callback for writing sample data
        delegate<void(void *, size_t)> on_write;

        // Notification when next subsong is about to start.
        delegate<void(int)> on_skip;

        // Notification when the song is paused/unpaused.
        delegate<void(bool)> on_pause;

        delegate<void(int, bool)> on_mute;

        delegate<void()> on_nextsong;
    private:
        bool load_gbs(const char *filepath, int starting_song = 0);
        bool load_m3u(const char *filepath);
        bool step_emulation();
        impl *m;

        friend void step_callback_handler(struct gbs *gbs, cycles_t cycles, const struct gbs_channel_status chan[], void *priv);
        friend void io_callback_handler(struct gbs *gbs, cycles_t cycles, uint32_t addr, uint8_t value, void *priv);
        friend void write_callback_handler(struct gbs *gbs, struct gbs_output_buffer *buf, void *priv);
        friend long nextsubsong_callback_handler(struct gbs *gbs, void *priv);
    };
}



#endif //GBS_OPUS_GBS_DRIVER_H
