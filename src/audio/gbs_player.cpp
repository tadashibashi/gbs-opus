#include "gbs_player.h"
#include "plugout_driver.h"

#include <gbs/libgbs.hpp>
#include <gbs/gbs_driver.h>
#include <vector>
#include <filesystem>

#include <SDL.h>
#include <iostream>

namespace gbs_opus
{
    struct opus_output_buffer
    {
        ~opus_output_buffer()
        {
            close();
        }

        void init()
        {
            close();
            buf.data = (int16_t *)malloc(buf.bytes);
        }

        void close()
        {
            if (buf.data)
                free(buf.data);
        }

        [[nodiscard]] size_t size() const
        {
            return buf.bytes;
        }

        gbs_output_buffer buf {
             .data = nullptr,
             .bytes = 8192,
             .pos = 0
        };
    };

    // GBS_PLAYER IMPLEMENTATION
    struct gbs_player::impl
    {
    public:
        impl() : driver{}, speed{1.f}, sample_rate{}, meta{}, mute{}, dtime{}, play_index{}
        {}

    public: // State
        bool paused;
        float speed; // multiplier; long=slow, short=fast
        long sample_rate; // cached for easy access
        int play_index;
    public: // Members
        //gbs *gbs;
        //opus_output_buffer buffer;
        //plugout_driver plugout;
        gbs_meta meta;
        bool mute[4];
        mutable displaytime dtime;
        gbs_driver driver;
        gbs_playlist playlist;
    };

    bool gbs_player::init(long sample_rate, int16_t buffer_size, plugout_type type)
    {
        m->driver.init(sample_rate, buffer_size, type);

        // Setup next song callback
        static std::function<void()> nextsongcb = [this]() {
            this->play_next();
        };
        m->driver.on_nextsong.add_listener(&nextsongcb);
        return true;
    }

    bool gbs_player::init(plugout_type p)
    {
        return init(44100, 2048, p);
    }


    bool gbs_player::load(const std::string &path)
    {
        if (!m->driver.load(path.c_str()))
            return false;

        gbs_meta meta;
        std::filesystem::path fspath(path);

        if (!meta.open(fspath.parent_path().string()))
            return false;

        m->meta = std::move(meta);

        // Setup playlist
        m->playlist.init(&m->meta);
        m->driver.set_subsoundinfo(m->meta);

        play_track(-128);
        return true;
    }


    void gbs_player::update()
    {
        m->driver.update();
    }

    void gbs_player::close()
    {
        m->driver.close();
    }

    bool gbs_player::is_running() const
    {
        return !m->driver.is_paused();
    }

    void gbs_player::toggle_pause()
    {
        m->driver.set_paused(!m->driver.is_paused());
    }

    void gbs_player::set_pause(bool pause)
    {
        m->driver.set_paused(pause);
    }

    bool gbs_player::is_paused() const {
        return m->driver.is_paused();
    }

    gbs_player::~gbs_player() {
        close();
        delete m;
    }

    bool gbs_player::play_track(int t)
    {
        m->driver.set_paused(false);

        if (t == -128) // TODO: Set this as a constant
        {
            auto song_index = m->driver.song_index();
            auto it = std::find_if(m->meta.tracks().begin(), m->meta.tracks().end(),
                                   [song_index](const gbs_meta::track_meta &track) { return track.track == song_index; });
            if (it != m->meta.tracks().end())
                m->playlist.goto_track(it - m->meta.tracks().begin());
            return m->driver.play_song(song_index);
        }
        else
            return m->driver.play_song(m->playlist.goto_track(t));
    }

    bool gbs_player::is_loaded() const {
        return m->driver.is_open();
    }

    const gbs_meta *gbs_player::meta() const {
        return &m->meta;
    }

    bool gbs_player::play_prev() {
        if (!is_loaded()) return false;

        m->playlist.prev_track();
        return play_track(m->playlist.index());
    }

    bool gbs_player::play_next() {
        if (!is_loaded()) return false;

        m->playlist.next_track();
        return play_track(m->playlist.index());
    }

    int gbs_player::current_track() const {
        return m->playlist.track();
    }

    size_t gbs_player::total_tracks() const {
        if (!is_loaded()) return -1;

        return m->playlist.size();
    }

    const displaytime &gbs_player::current_displaytime() const {
        auto &time = m->dtime;
        if (!is_loaded())
        {
            time = {0};
            return time;
        }

        long played = m->driver.tracktime_played();
        long total = m->driver.tracktime_total() - 1; // -1 for visual aesthetic

        time.played_min = played / 60;
        time.played_sec = played % 60;
        time.total_min = total / 60;
        time.total_sec = total % 60;

        return time;
    }

    void gbs_player::toggle_mute(int chan) {
        if (!is_loaded()) return;

        m->driver.set_mute(chan, !m->driver.is_mute(chan));
    }

    long gbs_player::get_mute(int chan) const {
        if (!is_loaded()) return -1;

        return m->driver.is_mute(chan);
    }

    void gbs_player::play_ticks(int ticks) {
        if (!is_loaded()) return;

        m->driver.play_ticks(ticks);
    }

    uint8_t gbs_player::io_peek(uint32_t addr) const
    {
        if (!is_loaded()) throw std::runtime_error("Error: Cannot io_peek if gbs is not loaded");

        return m->driver.io_peek(addr);
    }

    uint8_t *gbs_player::io_hack() {
        if (!is_loaded()) return nullptr;

        return m->driver.io_get();
    }

    gbs_player::gbs_player() : m(new impl)
    { }

    PlaylistMode gbs_player::playlist_mode() {
        return m->playlist.mode();
    }

    class gbs_driver *gbs_player::driver() const {
        return &m->driver;
    }

}