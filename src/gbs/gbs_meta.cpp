//
// Created by Aaron Ishibashi on 11/19/22.
//

#include "gbs_meta.h"
#include "meta_io.h"

#include "m3u.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include "include/nlohmann/json.hpp"
#include "libgbs.hpp"

bool gbs_opus::gbs_meta::open(const std::string &folder_path)
{
    std::filesystem::directory_entry entry(folder_path);
    if (!entry.exists())
    {
        std::cerr << "gbs_meta::open failed, item does not exist at path \"" <<
            folder_path << "\"\n";
        return false;
    }

    if (entry.is_regular_file())
    {
        if (entry.path().extension() == ".zgbs")
        {
            // open zgbs zip file
            throw "Opening zip files is not implemented yet!";
        }
        throw "Opening files is not implemented yet!";
    }
    else if (entry.is_directory())
    {
        // Metadata exists in folder already? Use it.

        if (std::filesystem::exists(folder_path + "/metadata.json"))
        {
            std::ifstream file(folder_path + "/metadata.json");
            populate(nlohmann::json::parse(file));
        }
        else // look for m3u files to parse data from them, or make generic data from .gbs
        {
            std::vector<m3u> m3us;
            for (const auto &e : std::filesystem::directory_iterator(entry))
            {
                if (e.path().filename() == "metadata.json")
                {
                    std::ifstream file(e.path());
                    populate(nlohmann::json::parse(file));
                    continue;
                }

                if (e.path().extension() == ".m3u")
                {
                    auto &m = m3us.emplace_back();
                    if (!m.open(e.path().string()))
                    {
                        std::cerr << "Warning: m3u at \"" << e.path() <<
                                  "\" did not load correctly. Skipping file.\n";
                        continue;
                    }
                }
            }

            // Look for corresponding .gbs
            gbs *gbs = nullptr;
            for (const auto &e : std::filesystem::directory_iterator(entry))
            {
                auto ext = e.path().extension();
                if (ext == ".gbs" || ext == ".gbr" || ext == ".gb" || ext == ".vgm")
                {
                    gbs = gbs_open(e.path().string().c_str());
                    break;
                }
            }

            if (gbs == nullptr)
            {
                std::cerr << "GBS metadata could not be found or created. "
                             "No .gbs file in folder: " << entry.path() << '\n';
                return false;
            }

            meta_io writer;

            // Generate info
            if (m3us.empty()) // Generate generic info if no m3u
            {
                if (!writer.init_generic(gbs))
                {
                    std::cerr << "Generic GBS metadata could not be created from file at " <<
                        entry.path() << '\n';
                    gbs_close(gbs);
                    return false;
                }
            }
            else // we have m3u's, parse metadata here
            {
                try {
                    std::sort(m3us.begin(), m3us.end(), [](const m3u &a, const m3u &b) {
                        return a.track_num < b.track_num;
                    });
                } catch (const std::exception &e)
                {
                    gbs_close(gbs);
                    std::cerr << "Exception while sorting m3u file vector: " << e.what() << '\n';
                    return false;
                }

                if (!writer.parse_m3u(gbs, m3us))
                {
                    std::cerr << "Error: problem while parsing m3us\n";
                    gbs_close(gbs);
                    return false;
                }
            }

            if (!writer.write_to_folder(entry.path().string()))
            {
                std::cerr << "Error: failed to write metadata.json file\n";
                gbs_close(gbs);
                return false;
            }

            m_gbs_version = gbs_get_version(gbs);

            populate(writer.data());
            gbs_close(gbs);
        }

        m_path = folder_path;

        return true;
    }


    /// Check for metadata.json file

    /// If none, load m3u files and create it with meta_writer obj

    /// Warn user if number of m3u do not match the song count.

    /// If no m3u's create generic template


    /// Check for album art



    return true;
}

void gbs_opus::gbs_meta::clear() {
    m_tracks.clear();
    m_title.clear();
    m_composer.clear();
    m_developer.clear();
    m_publisher.clear();
    m_copyright.clear();
    m_filetype.clear();

}

bool gbs_opus::gbs_meta::populate(const nlohmann::json &j)
{
    m_filetype = j.value("filetype", "");

    if (m_filetype != "GBS")
    {
        clear();
        std::cerr << "Not a valid gbs metadata file!\n";
        return false;
    }

    m_title = j.value("title", "");
    m_composer = j.value("composer", "");
    m_developer = j.value("developer", "");
    m_arranger = j.value("arranger", "");
    m_publisher = j.value("publisher", "");
    m_copyright = j.value("copyright", "");
    m_genre = j.value("genre", "");
    m_comment = j.value("comment", "");
    m_coverart = j.value("coverart", "");
    m_gbs_version = j.value("gbs_version", 0);

    std::vector<track_meta> tracks;
    if (!j.contains("tracks"))
    {
        std::cout << "Warning: no track information in this metadata json\n";
    }
    else
    {
        int debug_index = 1;
        for (auto track : j["tracks"])
        {
            auto &t = tracks.emplace_back();
            t.title = track.value("title", "");
            t.length = 0;
            if (track.contains("length"))
            {
                const auto &len = track["length"];
                t.length += len.value("min", 0) * 60;
                t.length += len.value("sec", 0);
            }

            t.fade = track.value("fade", 0);
            t.composer = track.value("composer", "");
            t.arranger = track.value("arranger", "");
            t.track = track.value("index", -1);

            t.comment = track.value("comment", "");
            t.artwork = track.value("artwork", "");

            if (t.track == -1)
            {
                std::cout << "Critical Warning: track titled \"" << t.title <<
                          "\" at position " << debug_index << ", is missing gbs track index field \"index\"\n";
            }

            if (t.title.empty())
            {
                std::cout << "Warning: track at position " << debug_index << ", is missing a title\n";
            }

            if (t.length == -1)
            {
                std::cout << "Warning: track at position " << debug_index << ", is missing length. "
                         "Setting it to 3 minutes\n";
                t.length = 60 * 3;
            }

            ++debug_index;
        }
    }

    m_tracks.swap(tracks);
    return true;
}
