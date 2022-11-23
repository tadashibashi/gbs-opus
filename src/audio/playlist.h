/**
 * This class helps with keeping track of varying playlist modes like linear, shuffle, gbs_file mode
 */
#ifndef GBS_OPUS_PLAYLIST_H
#define GBS_OPUS_PLAYLIST_H

#include <vector>
#include "gbs/gbs_meta.h"

namespace gbs_opus
{
    enum class PlaylistMode {
        Forward,
        Backward,
        Shuffle
    };

    class playlist {
    public:
        // Starts off in PlaylistMode::Forward. Use mode(PlaylistMode m) to set the mode.
        playlist() : m_mode{PlaylistMode::Forward}, m_list{}, m_index{}, m_count{} {}
        explicit playlist(PlaylistMode m) : m_mode{m}, m_list{}, m_index{}, m_count{} {}

        // Reset the internal index to 0, and sets up random shuffle if in Shuffle mode
        void init(int track_count);

        // Initialize the playlist with your own order
        void init(std::vector<uint8_t> order);

        // Get the playlist mode
        [[nodiscard]] virtual PlaylistMode mode() const { return m_mode; }

        // Set the playlist mode
        virtual void mode(PlaylistMode m);

        // Gets the play index. Use peek() to get the track number it points to.
        [[nodiscard]] virtual int index() const { return m_index; }

        // Gets the current track number the play list is pointing to.
        [[nodiscard]] virtual int track() const;

        // Goto forward one step in the playlist. Returns the resultant track.
        virtual int next_track();

        // Goes backward one step in the playlist. Returns the resultant track.
        virtual int prev_track();

        /// Clamps between lowest and highest indices, sends warning to console.
        virtual int goto_track(int index);
    private:
        void shuffle_list();
        [[nodiscard]] int sanitize_index(int index) const;
        std::vector<uint8_t> m_list;
        PlaylistMode m_mode;
        int m_index, m_count;
    };


    class gbs_playlist : playlist {
    public:
        gbs_playlist() : m_meta() { }
        explicit gbs_playlist(PlaylistMode m) : playlist(m), m_meta() { }
        bool init(const gbs_meta *meta);
        int next_track() override;
        int prev_track() override;
        int goto_track(int index) override;
        [[nodiscard]] int track() const override;
        [[nodiscard]] int index() const override;
        [[nodiscard]] PlaylistMode mode() const override;
        void mode(PlaylistMode m) override;
        [[nodiscard]] size_t size() const;
    private:
        const gbs_meta *m_meta;
    };
}



#endif //GBS_OPUS_PLAYLIST_H
