#ifndef GBS_OPUS_PLUGOUT_DRIVER_H
#define GBS_OPUS_PLUGOUT_DRIVER_H

#include <gbs/libgbs.hpp>

#include <string>

namespace gbs_opus
{
    enum class plugout_type {
        APP,
        SDL,
        ALSA,
        ALTMIDI,
        DEVDSP,
        DSOUND,
        IODUMPER,
        NAS,
        PULSE,
        STDOUT,
        MIDI,
        WAV,
        VGM,
    };

    class plugout_driver {
    public:
        plugout_driver();
    public:
        bool load();

        /// Gets the output plugin's name. Empty str if none loaded, or does not have one.
        [[nodiscard]] const char *name() const { return m_plugout.name; }
        /// Gets the output plugin's description. Empty str if none loaded, or does not have one.
        [[nodiscard]] const char *description() const { return m_plugout.description; }
        /// Gets the actual sample rate in Hz, of the output plugin.
        [[nodiscard]] long sample_rate() const { return m_sample_rate; }

        /// Gets the actual buffer size, in bytes, of the output plugin.
        [[nodiscard]] long buffer_size() const { return m_buffer_size; }

        /// Call before opening driver to set the requested sample rate.
        void request_buffer_size(long size) { m_req_buffersize = size; }

        /// Call before opening driver to set the requested sample rate.
        void request_sample_rate(long rate) { m_req_samplerate = rate; }

        void request_output_type(plugout_type type) { m_plugout_type = type; }

        /// Opens the plugout. Use request_* functions to set up params before opening.
        long call_open();

        void call_skip(int subsong) const
        {
            if (sound_skip) sound_skip(subsong);
        }

        void call_pause(int pause) const
        {
            if (sound_pause) sound_pause(pause);
        }

        void call_io(cycles_t cycles, uint32_t addr, uint8_t val) const
        {
            if (sound_io) sound_io(cycles, addr, val);
        }

        void call_step(const cycles_t cycles, const gbs_channel_status ch[]) const
        {
            if (sound_step) sound_step(cycles, ch);
        }

        void call_write(const void *buf, size_t count) const
        {
            if (sound_write) sound_write(buf, count);
        }

        void call_close() const
        {
            if (sound_close) sound_close();
        }

        [[nodiscard]] plugout_endian endian() const { return m_endian; }

    private:
        /* Initial open of plugout. */
        plugout_open_fn  sound_open;
        plugout_skip_fn  sound_skip;

        /* Notification that the player is paused/resumed. */
        plugout_pause_fn sound_pause;

        plugout_io_fn    sound_io;
        plugout_step_fn  sound_step;
        plugout_write_fn sound_write;
        plugout_close_fn sound_close;
        plugout_endian   m_endian;
        long m_sample_rate, m_buffer_size;
        long m_req_samplerate, m_req_buffersize;
        plugout_type m_plugout_type;
        output_plugin m_plugout;
    };
}



#endif //GBS_OPUS_PLUGOUT_DRIVER_H
