#ifndef GBS_OPUS_M3U_H
#define GBS_OPUS_M3U_H
#include <string>

namespace gbs_opus
{
    // Based off of the gbs m3u's from Zophar's domain
    class m3u {
    public:
        bool open(const std::string &path);
        void debug_log() const;
        int gbs_track_num;   // zero-based
        int track_num;
        std::string album_title;
        std::string composer;
        std::string track_title;
        std::string developer;
        std::string publisher;
        std::string copyright;
        std::string filename;
        int track_time; // seconds
        int fade_time;
    private:
        // Parses the middle string with '-' separators
        bool parse_info_str(const std::string &info);
        bool parse_time(const std::string &info, int *time);
        bool parse_copyright(const std::string &info, std::string &date, std::string &studio);
    };
}



#endif //GBS_OPUS_M3U_H
