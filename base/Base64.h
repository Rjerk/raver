#ifndef BASE64_H
#define BASE64_H

#include "noncopyable.h"
#include <string>

namespace base {

std::string base64Decode(const std::string& src);

}


#endif
