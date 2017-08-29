#include "../Timer.h"
#include <iostream>
#include <unistd.h>

int main()
{
    using namespace raver;

    Timer timer;
    timer.start();
    ::sleep(3);
    timer.end();

    std::cout << timer.elapsed() << std::endl;
}
