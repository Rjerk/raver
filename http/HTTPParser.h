#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "../base/noncopyable.h"

namespace raver {

class Buffer;
class HTTPRequest;

class HTTPParser : noncopyable {
public:
    enum RequestParseState {
        ExpectRequstLine,
        ExpectHeaders,
        ExpectBody,
        GotAll
    };

    HTTPParser(): state_(ExpectRequstLine) { }

    bool gotAll() { return state_ == GotAll; }
    bool parseRequest(Buffer* in, HTTPRequest* req);
    bool parseRequestLine(const char* begin, const char* end, HTTPRequest** req);
private:
    RequestParseState state_;
};


}

#endif
