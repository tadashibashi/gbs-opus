//
// Created by Aaron Ishibashi on 11/19/22.
//

#include "m3u.h"
#include <fstream>
#include <iostream>

enum class M3U_FIELD {
    FILE_NAME = 0,
    GBS_TRACK_NUM,
    GBS_INFO_STR,
    PUBLISHER,
    TRACK_LENGTH,
    EMPTY,
    LAST_NUM
};

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline std::string &trim(std::string &s) {
    rtrim(s);
    ltrim(s);
    return s;
}

bool gbs_opus::m3u::open(const std::string &path)
{
    auto file = std::fstream();
    file.open(path);

    if (!file.is_open())
    {
        std::perror("m3u file open failed");
        return false;
    }

    std::string m3u_str;
    if (!std::getline(file, m3u_str))
    {
        std::cerr << "Error: Failed to parse .m3u data string from file" << '\n';
        file.close();
        return false;
    }

    // TODO: don't mutate object's data until all is parsed. (also with parse_info_str())
    {
        size_t data_index = 0;
        size_t i = 0, last_i = 0;
        while(i <= m3u_str.length())
        {
            if (m3u_str[i] == ',' || i == m3u_str.length())
            {
                std::string data = m3u_str.substr(last_i, i-last_i);
                trim(data);

                // parse string here
                switch(M3U_FIELD(data_index))
                {
                    case(M3U_FIELD::FILE_NAME):

                        if (data.find_first_of("::GBS") == data.length()-1)
                        {
                            std::cerr << "Error: not a valid GBS M3U file!\n";
                            return false;
                        }

                        this->filename = data.substr(0, data.length()-5);
                        break;
                    case(M3U_FIELD::EMPTY):
                        if (!data.empty())
                        {
                            std::cerr << "Error: M3U empty field is not empty!\n";
                            return false;
                        }
                        break;
                    case(M3U_FIELD::GBS_INFO_STR):
                        parse_info_str(data);
                        break;
                    case(M3U_FIELD::GBS_TRACK_NUM):
                        this->gbs_track_num = std::stoi(data);
                        break;
                    case(M3U_FIELD::PUBLISHER):
                        this->publisher = data;
                        break;
                    case(M3U_FIELD::TRACK_LENGTH):
                        // parse time as seconds
                        int t_time;
                        if (!this->parse_time(data, &t_time))
                        {
                            std::cerr << "Error: time is in an invalid format\n";
                            return false;
                        }
                        this->track_time = t_time;
                        break;
                    case(M3U_FIELD::LAST_NUM):
                        this->fade_time = std::stoi(data);
                        break;
                    default:
                        std::cout << "Warning: Unknown m3u field at index " <<
                            data_index << '\n';
                        break;
                }


                last_i = i + 1;
                ++data_index;
            }
            ++i;
        }
    }

    return true;
}

bool gbs_opus::m3u::parse_info_str(const std::string &info)
{
    if (info.empty())
        return false;

    std::string t_track_title, t_composer, t_album_title, t_copyright, t_studio;

    int i = 0, last_i = 0, data_index = 0;
    while (i <= info.length())
    {
        if (i > 0)
        {
            // Check if data should be parsed
            bool should_parse = false;
            if (info[i] == ' ' && i < info.length()-1)
            {
                if (info[i+1] == '-')
                {
                    should_parse = true;
                }
            }
            else if (info[i] == '\\' || i >= info.length())
                should_parse = true;

            if (should_parse)
            {
                // Parse data
                std::string data = info.substr(last_i, i-last_i);
                trim(data);
                switch(data_index)
                {
                    case 0: // Track title
                        t_track_title = data;
                        break;
                    case 1: // Composer
                        t_composer = data;
                        break;
                    case 2: // Album title
                        t_album_title = data;
                        break;
                    case 3: // Copyright
                        if(!parse_copyright(data, t_copyright, t_studio))
                        {
                            std::cout << "Error: failed to parse copyright in "
                                         "GBS Info portion of m3u\n";
                            return false;
                        }
                        break;
                    default:
                        std::cout << "Warning: GBS Info portion of m3u has "
                                     "more fields than expected\n";
                        break;
                }

                // Increment for next data
                ++data_index;
                last_i = i + 3; // +3 starts at beginning of next data (" - " delimiter)
                i += 3;
            }
        }

        ++i;
    }

    // Error check
    if (data_index < 3)
    {
        std::cerr << "Error: Could not parse required fields of GBS info portion in m3u\n";
        return false;
    }

    // Success, commit vars
    this->album_title = t_album_title;
    this->composer = t_composer;
    this->copyright = t_copyright;
    this->track_title = t_track_title;
    this->developer = t_studio;

    return true;
}

bool gbs_opus::m3u::parse_time(const std::string &info, int *time)
{
    if (!time)
        return false;
    int res = 0;

    auto colon_index = info.find(':');
    if (colon_index >= info.length()-1)
        return false;

    res += std::stoi(info.substr(0, colon_index)) * 60;
    res += std::stoi(info.substr(colon_index+1));

    *time = res;
    return true;
}

void gbs_opus::m3u::debug_log() const
{
    std::cout << "----- M3U Data -----\n";
    std::cout <<
              "File name  : " << filename << '\n' <<
              "Album title: " << album_title << '\n' <<
              "Composer   : " << composer << '\n' <<
              "Track title: " << track_title << '\n' <<
              "Track time : " << track_time / 60 << ':' << track_time % 60 << '\n' <<
              "Fade time  : " << fade_time << " seconds\n" <<
          "GBS track #: " << gbs_track_num << '\n' <<
          "Developer  : " << developer << '\n' <<
          "Publisher  : " << publisher << '\n' <<
          "Copyright  : " << copyright << '\n';
}

bool gbs_opus::m3u::parse_copyright(const std::string &info,
                                    std::string &date, std::string &studio)
{
    int studio_start = -1;
    for (int i = 0; i < info.length(); ++i)
    {
        if (info[i] == ' ' && i < info.length()-1)
        {
            studio_start = i + 1;
            break;
        }
    }

    if (studio_start == -1 || info.length() - studio_start < 2)
    {
        std::cout << "Warning: could not find studio name\n";
        return false;
    }

    date = info.substr(0, studio_start - 1);
    studio = info.substr(studio_start);

    return true;
}
