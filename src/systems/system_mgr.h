//
// Created by Aaron Ishibashi on 11/16/22.
//

#ifndef GBS_OPUS_SYSTEM_MGR_H
#define GBS_OPUS_SYSTEM_MGR_H

#include <vector>
#include <map>
#include "system.h"

namespace gbs_opus
{
    class system_mgr {
    public:
        void load(std::vector<system *> systems);
        void init();
        void close();
    private:
        std::map<std::string, system *> m_map;
        std::vector<updatable *> m_updatable;
        std::vector<renderable *> m_renderable;
    };
}



#endif //GBS_OPUS_SYSTEM_MGR_H
