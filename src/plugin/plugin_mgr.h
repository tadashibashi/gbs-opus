#ifndef GBS_OPUS_PLUGIN_MGR_H
#define GBS_OPUS_PLUGIN_MGR_H
#include "plugin.h"

#include <vector>


namespace gbs_opus
{
    class plugin_mgr {
    public:
        plugin_mgr();
        ~plugin_mgr();

        /**
         * Load a plugin from the folder path
         * @param path
         * @return
         */
        bool load(const std::string &path);

        /**
         * Scan a folder for plugins
         * TODO: Scan folder for zipped plugs
         * @param dir
         */
        void scan(const std::string &dir);

    private:
        std::vector<plugin> m_active;
        std::vector<std::string> m_avail;
    };
}



#endif //GBS_OPUS_PLUGIN_MGR_H
