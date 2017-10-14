#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "HTTPRequest.h"

namespace raver {

class Buffer;
class HTTPRequest;

class HTTPParser {
public:
    enum RequestParseState {
        ExpectRequstLine,
        ExpectHeaders,
        ExpectBody,
        GotAll
    };

    HTTPParser(): state_(ExpectRequstLine) { }

    ~HTTPParser() { }

    bool gotAll() { return state_ == GotAll; }

    bool parseRequest(Buffer* in);

    const HTTPRequest& request() const { return request_; }

private:
    bool parseRequestLine(const char* begin, const char* end);

private:
    RequestParseState state_;
    HTTPRequest request_;
};


}

#endif
