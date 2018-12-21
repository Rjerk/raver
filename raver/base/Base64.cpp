#include "Base64.h"
#include <bitset>
#include "Logger.h"

namespace raver {

namespace detail {

bool isBase64(unsigned char c) { return isalnum(c) || c == '+' || c == '/'; }
static const char base64_digits[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63, 52,
    53, 54, 55, 56, 57, 58, 59, 60, 61,
    //             +(43 ascii)  /   -1   1   2   3   4   5   6   7   8   9
    0, 0, 0, -1, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
    //                    a  b  c  d  e ...
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26, 27,
    28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
    47, 48, 49, 50, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

}  // namespace detail

std::string base64Decode(const std::string& src) {
  if (src.size() % 4) {  // invalid decoded base64 string.
    LOG_ERROR << "base64 decode failed, invalid string len";
    return "";
  }

  // a:aa -> 011000 010011 101001 100001 011000 01xxxx xxxxxx xxxxxx -->
  // YTphYQ==
  int n = 0;
  for (auto it = src.rbegin(); *it == '='; ++it) {
    ++n;
  }  // n = 2.

  auto padding_bit_len = (src.size() - n) * 6 % 8;  // 01xxxx -> 4.
  auto origin_bit_len =
      (src.size() - n) * 6 - padding_bit_len;  // 48-12-4 = 32.

  std::string binary_str;
  for (auto ch : src) {
    if (ch == '=') continue;

    if (detail::isBase64(ch)) {
      binary_str +=
          std::bitset<6>(detail::base64_digits[static_cast<unsigned int>(ch)])
              .to_string();
    } else {
      LOG_ERROR << "Invalid decoded string.";
      return "";
    }
  }

  // 01100001 00111010 01100001 01100001
  std::string result = std::string(binary_str, 0, origin_bit_len);

  std::string dst;
  for (size_t i = 0; i < origin_bit_len / 8; ++i) {
    dst +=
        static_cast<char>(std::stoi(std::string(result, i * 8, 8), nullptr, 2));
  }

  LOG_TRACE << "decode: " << dst;
  return dst;
}

}  // namespace raver
