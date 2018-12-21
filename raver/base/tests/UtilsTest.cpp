#include "raver/base/Utils.h"
#include <iostream>

int main()
{
    std::string type;
    utils::getContentType("txt", type);
    std::cout << type << std::endl;
    utils::getContentType("png", type);
    std::cout << type << std::endl;
    utils::getContentType("htm", type);
    std::cout << type << std::endl;
}
