#include "raver/base/Base64.h"
#include <iostream>

int main()
{
    std::cout << raver::base64Decode("YTph") << std::endl; // a:a
    std::cout << raver::base64Decode("YTphYQ==") << std::endl; // a:aa
    std::cout << raver::base64Decode("YTphYWE=") << std::endl; // a:aaa
    std::cout << raver::base64Decode("YTphYWFh") << std::endl; // a:aaaa
}
