#include "raver/base/TimeStamp.h"
#include <iostream>
#include <unistd.h>

int main()
{
    using namespace raver;

    auto before = TimeStamp::getTicks();

    ::sleep(3);

    auto duration = TimeStamp::getTicks() - before;

    auto secs = static_cast<double>(duration) / TimeStamp::ticksPerSecond();

    std::cout << secs << std::endl;

}
