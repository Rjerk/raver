#include "HTTPRequest.h"

namespace raver {

HTTPRequest::HTTPRequest()
    : method_(Invalid), version_(Unknown)
{
}

void HTTPRequest::addHeader(const char* begin, const char* colon, const char* end)
{
    std::string field(begin, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
        ++colon;
    }

    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size()-1])) {
        value.resize(value.size()-1);
    }

    headers_[field] = value;
}

bool HTTPRequest::setMethod(const char* beg, const char* end)
{
    std::string m(beg, end);

    if (m == "GET") {
        method_ = Method::Get;
    } else if (m == "POST") {
        method_ = Method::Post;
    } else if (m == "Head") {
        method_ = Method::Head;
    } else if (m == "Put") {
        method_ = Method::Put;
    } else if (m == "Delete") {
        method_ = Method::Delete;
    } else {
        method_ = Method::Invalid;
    }
}

}
