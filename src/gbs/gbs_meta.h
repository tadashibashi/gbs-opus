//
// Created by Aaron Ishibashi on 11/19/22.
//

#ifndef GBS_OPUS_GBS_META_H
#define GBS_OPUS_GBS_META_H
#include <string>
#include <vector>
#include <map>
#include "m3u.h"
#include "include/nlohmann/json_fwd.hpp"

namespace gbs_opus
{
    class gbs_meta {
    public:
        struct track_meta
        {
            std::string title;
            std::string composer; // blank unless different from main composer
            std::string arranger; // blank unless different from main arranger
            std::string artwork;
            std::string comment;
            int track;  // gbs track num
            int length; // seconds
            int fade;   // seconds
        };

        // Opens file at location and parses metadata. Returns true on success,
        // and false otherwise.
        bool open(const std::string &folder_path);

        // Cleans all data for this object to be reused
        void clear();

        // Retrieves the number of songs in the playlist
        [[nodiscard]] size_t num_tracks() const { return m_tracks.size(); }

        [[nodiscard]] const std::vector<track_meta> &tracks() const { return m_tracks; }

        [[nodiscard]] const std::string &title() const { return m_title; }
        [[nodiscard]] const std::string &composer() const { return m_composer; }
        [[nodiscard]] const std::string &arranger() const { return m_arranger; }
        [[nodiscard]] const std::string &developer() const { return m_developer; }
        [[nodiscard]] const std::string &publisher() const { return m_publisher; }
        [[nodiscard]] const std::string &copyright() const { return m_copyright; }
        [[nodiscard]] const std::string &genre() const { return m_genre; }
        [[nodiscard]] const std::string &comment() const { return m_comment; }
        [[nodiscard]] const std::string &coverart() const { return m_coverart; }
        [[nodiscard]] const std::string &path() const { return m_path; }
        [[nodiscard]] uint8_t gbs_version() const { return m_gbs_version; }

    private:
        bool populate(const nlohmann::json &j);
        std::vector<track_meta> m_tracks;
        std::string m_title, m_composer, m_arranger, m_developer, m_path,
            m_publisher, m_copyright, m_filetype, m_genre, m_comment, m_coverart;
        uint8_t m_gbs_version;
    };
}



#endif //GBS_OPUS_GBS_META_H
