#include "app.h"

#ifdef _WIN32
#include <SDL_main.h>
#endif

#include <sol/sol.hpp>
#include <SDL.h>
int main(int argc, char *argv[])
{
    char *basepath = SDL_GetBasePath();

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    lua.script("opus = {}");
    std::cout << "package path is " << lua["package"]["path"].get<std::string>() << '\n';
    lua["package"]["path"] = std::string(basepath) + "res/?.lua";
    std::cout << "package path is " << lua["package"]["path"].get<std::string>() << '\n';
    lua["opus"]["basepath"] = basepath;
    int i = 100;

    lua.set_function("increase", [&i](){++i;});

    std::cout << "i is:     " << i << '\n';
    lua.script("increase()");
    std::cout << "i is now: " << i << '\n';
    lua.script("print(opus.basepath) print(...)");
    lua.script("require('plug-driver')");
    SDL_free(basepath);

    auto a = new gbs_opus::app;
    a->run();

    delete a;
    return 0;
}
