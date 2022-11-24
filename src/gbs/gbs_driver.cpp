#include "gbs_driver.h"
#include "impl/gbs_driver_impl.h"
#include "m3u.h"
#include "gbs_meta.h"


#include <gbs/libgbs.hpp>
#include <cassert>
#include <iostream>
#include <filesystem>

static const int GbsMaxChans = 4;

namespace gbs_opus
{


#pragma region Callback Headers
    // Callback to update our note display
    void step_callback_handler(struct gbs *gbs, cycles_t cycles, const struct gbs_channel_status chan[], void *priv);
    void io_callback_handler(struct gbs *gbs, cycles_t cycles, uint32_t addr, uint8_t value, void *priv);
    void write_callback_handler(struct gbs *gbs, struct gbs_output_buffer *buf, void *priv);
    //void pause_callback_handler(struct gbs *gbs, int paused, void *priv);
    //long skip_callback_handler(struct gbs *gbs, void *priv);
    long nextsubsong_callback_handler(struct gbs *gbs, void *priv);
#pragma endregion

#pragma region Initialization & Closing
    gbs_driver::gbs_driver() : m(new impl) {

    }

    gbs_driver::~gbs_driver() {
        delete m;
    }

    bool gbs_driver::init(long sample_rate, int16_t buffer_size, plugout_type type) {
        // Load plugout and open it
        m->plugout.request_output_type(type);
        m->plugout.request_buffer_size(buffer_size);
        m->plugout.request_sample_rate(sample_rate);

        m->plugout.load();
        m->plugout.call_open();

        m->init_buffer(m->plugout.buffer_size());

        return true;
    }

    bool gbs_driver::load_gbs(const char *filepath, int starting_song) {
        struct gbs *temp_gbs = gbs_open(filepath);
        if (!temp_gbs)
        {
            std::cerr << "Error opening gbs file at " << filepath << '\n';
            return false;
        }

        // Set gbs callbacks here
        gbs_set_step_callback(temp_gbs, step_callback_handler, this);
        gbs_set_io_callback(temp_gbs, io_callback_handler, this);
        gbs_set_sound_callback(temp_gbs, write_callback_handler, this);
        gbs_set_nextsubsong_cb(temp_gbs, nextsubsong_callback_handler, this);

        gbs_configure_output(temp_gbs, &m->buffer, m->plugout.sample_rate());
        gbs_configure(temp_gbs, starting_song, 120 * 3, 5, 0, 2);

        /// TODO: Make filter configurable
        gbs_set_filter(temp_gbs, gbs_filter_type::FILTER_DMG);

        if (m->gbs)
            gbs_close(m->gbs);

        m->gbs = temp_gbs;

        return true;
    }

    void gbs_driver::close() {
        m->plugout.call_close();
        m->close();
    }

#pragma endregion

#pragma region Setters & Getters
    int gbs_driver::gbs_version() const {
        if (!m->gbs) return 0;
        assert(m->gbs);
        return gbs_get_version(m->gbs);
    }

    bool gbs_driver::is_mute(int chan) const {
        assert(m->gbs);
        assert(chan >= 0 && chan < GbsMaxChans);

        auto status = gbs_get_status(m->gbs);
        return status->ch[chan].mute;
    }

    void gbs_driver::set_mute(int chan, bool mute) {
        assert(m->gbs);
        assert(chan >= 0 && chan < GbsMaxChans);

        if (is_mute(chan) != mute)
            gbs_toggle_mute(m->gbs, chan);
    }

    size_t gbs_driver::song_count() const {
        assert(m->gbs);

        return gbs_get_status(m->gbs)->songs;
    }

    size_t gbs_driver::buffer_size() const {
        return (size_t)m->buffer.bytes;
    }

    int16_t *gbs_driver::buffer() {
        return m->buffer.data;
    }

    bool gbs_driver::is_open() const {
        return static_cast<bool>(m->gbs);
    }

    float gbs_driver::tracktime_played() const
    {
        if (!m->gbs) return 0;

        auto time = (long double)gbs_get_status(m->gbs)->ticks;

        if (time == 0)
        {
            return 0;
        }
        else
        {
            return time / GBHW_CLOCK;
        }
    }

    float gbs_driver::tracktime_total() const
    {
        assert(m->gbs);
        auto time = (long double)gbs_get_status(m->gbs)->subsong_len;

        if (time == 0)
        {
            return 0;
        }
        else
        {
            return (float)time;
        }
    }

    void gbs_driver::set_paused(bool paused) {
        if (paused == m->paused) return;

        m->plugout.call_pause(paused);
        m->paused = paused;
    }

    const plugout_driver &gbs_driver::plugout() const
    {
        return m->plugout;
    }

    const char *gbs_driver::filetype() const {
        assert(m->gbs);
        switch(gbs_get_filetype(m->gbs))
        {
            case 0: return "GBS";
            case 1: return "GBR";
            case 2: return "GB";
            case 3: return "VGM";
            default: return "Unknown";
        }
    }

    gbs *gbs_driver::gbs() {
        return m->gbs;
    }

    bool gbs_driver::step_emulation() {
        assert(m->gbs);

        if (!m->paused)
            return gbs_step(m->gbs, m->refresh_delay);

        nanosleep(&m->pause_wait_time, nullptr);
        return true;
    }

    void gbs_driver::update() {
        if (m->gbs)
            step_emulation();
    }

    static const double OneTick = 1000.0 / 59.7;
    bool gbs_driver::play_ticks(int ticks)
    {
        assert(m->gbs);
        set_paused(true);

        return gbs_step(m->gbs, (long)(OneTick * ticks + 1));
    }

    uint8_t gbs_driver::io_peek(uint16_t addr) const
    {
        assert(m->gbs);
        return gbs_io_peek(m->gbs, addr);
    }

    uint8_t *gbs_driver::io_get()
    {
        assert(m->gbs);
        return const_cast<uint8_t *>(gbs_io_get(m->gbs));
    }

    bool gbs_driver::is_paused() const {
        return m->paused;
    }

    bool gbs_driver::load_m3u(const char *path) {
        // Find the filename the m3u is pointing to
        m3u meta;
        if (!meta.open(path)) return false;

        std::filesystem::path fspath(path);

        std::filesystem::directory_entry entry(fspath.parent_path().string() + "/" + meta.filename);
        if (entry.exists() && entry.is_regular_file())
        {
            return load_gbs(entry.path().c_str(), meta.gbs_track_num);
        }

        std::cerr << "Associated gbs file could not be found for m3u at " <<
                  entry.path() << '\n';
        return false;
    }

    bool gbs_driver::load(const char *filepath)
    {
        auto ext = std::filesystem::path(filepath).extension();

        bool result = false;
        if (ext == ".gbs" || ext == ".gbr" || ext == ".vgm" || ext == ".gb")
        {
            result = load_gbs(filepath);
        }
        else if (ext == ".m3u")
        {
            result = load_m3u(filepath);
        }

        if (!result)
            return false;

        return true;
    }

    bool gbs_driver::play_song(int song_index)
    {
        if (!m->gbs) return false; // for protection

        on_skip.try_invoke(song_index);
        m->plugout.call_skip(song_index);

        return gbs_init(m->gbs, song_index);
    }

    void gbs_driver::set_subsoundinfo(const gbs_meta &meta)
    {
        assert(m->gbs);

        std::vector<gbs_subsong_info> info;
        info.reserve(song_count());
        info.assign(song_count(), {});
        for (auto &track : meta.tracks())
        {
            auto &i = info[track.track];
            i.title = track.title.c_str();
            i.len = track.length;
            i.fadeout = track.fade;
        }

        gbs_set_subsong_info(m->gbs, info.data(), info.size());
    }


#pragma endregion

#pragma region Callback Definitions
// ========= Bit Helpers =================================
    static void swap_endian(struct gbs_output_buffer *buf)
    {
        unsigned long i;

        for (i=0; i<buf->bytes/sizeof(uint16_t); i++) {
            uint16_t x = buf->data[i];
            buf->data[i] = ((x & 0xff) << 8) | (x >> 8);
        }
    }

    // ========== Callback Handlers =========

    void step_callback_handler(struct gbs *gbs, cycles_t cycles, const struct gbs_channel_status *chan, void *priv)
    {
        auto driver = static_cast<gbs_driver *>(priv);
        driver->m->plugout.call_step(cycles, chan);
        driver->on_step.try_invoke(cycles, chan);
    }

    void write_callback_handler(struct gbs *gbs, struct gbs_output_buffer *buf, void *priv)
    {
        auto driver = static_cast<gbs_driver *>(priv);

        if (driver->plugout().endian() != PLUGOUT_ENDIAN_NATIVE)
            swap_endian(buf);

        size_t bufsize = buf->pos * 2 * sizeof(int16_t);

        driver->on_write.try_invoke(buf->data, bufsize);
        driver->plugout().call_write(buf->data, bufsize);

        buf->pos = 0;
    }

    void io_callback_handler(struct gbs *gbs, cycles_t cycles, uint32_t addr, uint8_t value, void *priv)
    {
        auto driver = static_cast<gbs_driver *>(priv);

        driver->plugout().call_io(cycles, addr, value);

        // Currently no io callback exposed in gbs_player... maybe add it later?
    }

    long nextsubsong_callback_handler(struct gbs *gbs, void *priv)
    {
        auto driver = static_cast<gbs_driver *>(priv);

        driver->on_nextsong.try_invoke();
        return 0;
    }

    int gbs_driver::song_index() const {
        if (!m->gbs) return -1;

        return gbs_get_status(m->gbs)->subsong;
    }

#pragma endregion



}


