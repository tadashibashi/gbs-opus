#include "app.h"

int main()
{
    auto a = new gbs_opus::app;
    a->run();

    delete a;
    return 0;
}
