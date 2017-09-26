#include "Base64.h"
#include <map>

namespace base {

namespace detail {

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char base64_digits[] = {


};

}

std::string base64Decode(const std::string& src)
{
    if (src.size() % 4) { // invalid decoded base64 string.
        return "";
    }

    // a:aa -> 011000 010011 101001 1000001 011000 01xxxx xxxxxx xxxxxx --> YTphYQ==
    int n = 0;
    for (auto it = src.rbegin(); *it == '='; ++it) {
        ++n;
    }

    // encoded_str_len x 4 / 3 = decoded_str_len - padding_=_for_decode_str
    auto encoded_len = 0;
    std::string result(src.size()/4*3 + 1, char());
    auto p = src.data();

    while (*p) {
        char a = detail::base64_digits[static_cast<unsigned char>(*(p++))];
        char b = detail::base64_digits[static_cast<unsigned char>(*(p++))];
        char c = detail::base64_digits[static_cast<unsigned char>(*(p++))];
        char d = detail::base64_digits[static_cast<unsigned char>(*(p++))];

    }

    return result;
}

}
