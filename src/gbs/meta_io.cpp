#include "meta_io.h"
#include <filesystem>
#include <iostream>
#include "include/nlohmann/json.hpp"
#include "SDL_rwops.h"
#include <fstream>

#include "libgbs.hpp"
#include "gbs_meta.h"

using namespace nlohmann;

struct gbs_opus::meta_io::impl
{
    json j;
};

bool gbs_opus::meta_io::parse_m3u(gbs *gbs, const std::vector<m3u> &meta)
{
    if (!gbs)
    {
        std::cerr << "Error: meta_writer::from_m3u: passed gbs was null\n";
        return false;
    }
    if (meta.empty())
    {
        std::cerr << "Error: meta_writer::from_m3u: no m3u metadata was passed\n";
        return false;
    }

    auto gbs_meta = gbs_get_metadata(gbs);
    if (!gbs_meta)
    {
        std::cerr << "Error: meta_writer::from_m3u: failed to get metadata "
                     "from passed gbs\n";
        return false;
    }

    try {
        // Header data
        json j;
        j["title"] = gbs_meta->title;
        j["composer"] = gbs_meta->author;
        j["developer"] = meta[0].developer;
        j["publisher"] = meta[0].publisher;
        j["copyright"] = gbs_meta->copyright;
        j["filetype"] = "GBS";
        j["gbs_version"] = gbs_get_version(gbs);

        // Put track data into json array
        j["tracks"] = json::array();
        for (auto &m: meta)
        {
            auto obj = json::object();
            obj["title"] = m.track_title;
            obj["length"] = json::object({{"min", m.track_time/60},
                                          {"sec", m.track_time%60}});
            obj["fade"] = m.fade_time;
            obj["composer"] = m.composer;
            obj["index"] = m.gbs_track_num;
            j["tracks"].emplace_back(obj);
        }

        // Sucess! Commit result
        m_impl->j.swap(j);
        std::cout << m_impl->j.dump() << '\n'; // DEBUG LOG
        return true;
    }

    // Catch errors.....
    catch (const json::exception &e)
    {
        std::cerr << "Error: meta_writer::from_m3u: json exception, failed to create json: " <<
            e.what() << '\n';
        return false;
    }
    catch(...)
    {
        std::cerr << "Error: meta_writer::from_m3u: json exception, failed to create json\n";
        return false;
    }
}

void
gbs_opus::meta_io::close()
{
    m_impl->j.clear();
}

const nlohmann::json &
gbs_opus::meta_io::data() const
{
    return m_impl->j;
}

bool
gbs_opus::meta_io::write_to_folder(const std::string &filepath)
{
    // TODO: Support writing to zipped .zgbs format

    if (!std::filesystem::directory_entry(filepath).is_directory())
    {
        std::cerr << "Error: meta_writer can only write to a folder. " <<
            filepath << " is not folder\n";
        return false;
    }

    SDL_RWops *rw = SDL_RWFromFile((filepath + MetadataFilename).c_str(), "w");
    if (!rw)
    {
        std::cerr << "Error: meta_writer write cancelled, problem with "
                     "SDL_RWFromFile: " << SDL_GetError() << '\n';
        return false;
    }

    auto jstr = m_impl->j.dump(4);
    if (SDL_RWwrite(rw, jstr.data(), jstr.size(), 1) != 1)
    {
        std::cerr << "Error: meta_writer write cancelled, problem with "
                     "SDL_RWwrite: " << SDL_GetError() << '\n';
        return false;
    }

    if (SDL_RWclose(rw) != 0)
    {
        std::cout << "Warning: meta_writer RWops failure while closing\n";
    }

    return true;
}

bool gbs_opus::meta_io::init_generic(gbs *gbs) {

    if (!gbs)
    {
        std::cerr << "meta_writer::init_generic failed: gbs is null\n";
        return false;
    }

    auto gbsmeta = gbs_get_metadata(gbs);
    if (!gbsmeta)
    {
        std::cerr << "meta_writer::init_generic failed: problem while "
                     "getting gbs metadata\n";
        return false;
    }

    auto ssdata = gbs_get_subsong_info(gbs);
    if (!ssdata)
    {
        std::cerr << "meta_writer::init_generic failed: problem while "
                     "getting gbs subsong data\n";
        return false;
    }

    auto status = gbs_get_status(gbs);
    if (!status)
    {
        std::cerr << "meta_writer::init_generic failed: problem while "
                     "getting gbs status\n";
        return false;
    }

    // MAIN INFO
    // title:    Get from GBS metadata
    // composer: Get from GBS metadata
    // developer:
    // publisher:
    // copyright: Get from GBS metadata
    // genre: ""
    //
    json j;
    j["title"] = gbsmeta->title;
    j["composer"] = gbsmeta->author;
    j["copyright"] = gbsmeta->copyright;
    j["developer"] = "";
    j["publisher"] = "";
    j["genre"] = "";
    j["gbs_version"] = gbs_get_version(gbs);

    // TRACKS
    // title : "Track 1"
    // length : { min: 3, sec: 0 }
    // fade : 10
    //
    j["tracks"] = json::array();
    int track_num = 0;
    for (auto ssi = ssdata, end = ssdata + status->songs; ssi != end; ++ssi)
    {
        auto obj = json::object({
            {"title", "Track " + std::to_string(++track_num)},
            {"fade", 0}
        });
        obj["length"] = json::object({{"min", 3, "sec", 0}});
        obj["index"] = track_num;
        j["tracks"].emplace_back(obj);
    }

    // Success, commit
    m_impl->j.swap(j);
    return true;
}

gbs_opus::meta_io::meta_io() : m_impl(new impl) {

}

gbs_opus::meta_io::~meta_io() {
    delete m_impl;
}

bool gbs_opus::meta_io::open_json(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Error: meta_io could not open json file\n";
        return false;
    }

    try {
        auto j = nlohmann::json::parse(file);
        m_impl->j.swap(j);
        return true;
    } catch (const json::exception &e)
    {
        std::cerr << "Error: in meta_io: while parsing json file: " << e.what() << '\n';
    } catch (...)
    {
        std::cerr << "Error: in meta_io: unknown error while parsing json file\n";
    }

    return false;
}
