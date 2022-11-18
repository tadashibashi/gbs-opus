#ifndef GBS_OPUS_ACTIONS_H
#define GBS_OPUS_ACTIONS_H
#include <string>
#include <functional>

namespace gbs_opus::actions
{
    void init();
    extern std::function<std::string()> open_file_dialog;
}


#endif //GBS_OPUS_ACTIONS_H
