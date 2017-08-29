#include "../TimeStamp.h"
#include <iostream>
#include <unistd.h>

int main()
{
    using namespace raver;

    auto before = TimeStamp::getTicks();

    ::sleep(3);

    auto duration = TimeStamp::getTicks() - before;

    double secs = duration / TimeStamp::ticksPerSecond();

    std::cout << secs << std::endl;

}
