#include <raver/base/Utils.h>
#include <iostream>

int main()
{
    std::string type;
    utils::GetContentType("txt", type);
    std::cout << type << '\n';

    std::cout << utils::ReadFile("Makefile") << std::endl;
}
