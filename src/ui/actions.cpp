//
// Created by Aaron Ishibashi on 11/16/22.
//

#include "actions.h"
#include <systems.h>

namespace gbs_opus
{
    std::function<std::string()> actions::open_file_dialog;

    void actions::init()
    {
        actions::open_file_dialog = systems::open_file_dialogue;
    }
}

