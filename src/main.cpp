#include "app.h"
#include <thread>

void run_app(gbs_opus::app *a)
{
    a->run();
}

int main()
{
    auto a = new gbs_opus::app;
    a->run();

    delete a;
    return 0;
}
