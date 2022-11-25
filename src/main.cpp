#include "app.h"

#ifdef _WIN32
#include <SDL_main.h>
#endif

int main(int argc, char *argv[])
{
    auto a = new gbs_opus::app;
    a->run();

    delete a;
    return 0;
}
