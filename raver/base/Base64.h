#ifndef BASE64_H
#define BASE64_H

#include <string>
#include "noncopyable.h"

namespace raver {

std::string base64Decode(const std::string& src);
}

#endif
