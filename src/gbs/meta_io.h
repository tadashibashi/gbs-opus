//
// Created by Aaron Ishibashi on 11/20/22.
//

#ifndef GBS_OPUS_META_IO_H
#define GBS_OPUS_META_IO_H
#include "m3u.h"
#include <vector>

#include "include/nlohmann/json_fwd.hpp"
struct gbs;

namespace gbs_opus
{
    class meta_io {
        struct impl;
    public:
        meta_io();
        ~meta_io();

        // Create json data via a vector of m3u. Returns true on success,
        // and false otherwise. On true, object is ready to write.
        bool parse_m3u(gbs *gbs, const std::vector<m3u> &meta);

        // Load a generic
        bool init_generic(gbs *gbs);
        bool write_to_folder(const std::string &filepath);

        bool open_json(const std::string &filepath);

        [[nodiscard]] const nlohmann::json &data() const;
        void close();
    private:
        impl *m_impl;
    };
}



#endif //GBS_OPUS_META_IO_H
