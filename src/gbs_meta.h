//
// Created by Aaron Ishibashi on 11/19/22.
//

#ifndef GBS_OPUS_GBS_META_H
#define GBS_OPUS_GBS_META_H
#include <string>
#include <vector>
#include <map>
#include <m3u.h>

namespace gbs_opus
{
    class gbs_playlist {
    public:
        bool open(const std::string &folder_path);

        void clear() { m_playlist.clear(); m_data.clear(); }

        // Retrieves the number of songs in the playlist
        [[nodiscard]] size_t size() const { return m_playlist.size(); }
        [[nodiscard]] const m3u &by_list(size_t index) { return m_playlist[index]; }
        [[nodiscard]] const m3u &by_file(size_t index) { return m_data[index]; }
    private:
        std::vector<m3u> m_playlist;
        std::vector<m3u> m_data;
    };
}



#endif //GBS_OPUS_GBS_META_H
