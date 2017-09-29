#include <unistd.h>
#include <cstring>
#include <iostream>
#include <errno.h>

int main()
{
    ::execl("../www/cgi-bin/print_env.py", /*"../www/cgi-bin/print_env.py",*/ NULL);
    std::cout << ::strerror(errno) << std::endl;
}
