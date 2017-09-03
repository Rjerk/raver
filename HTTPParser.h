#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "noncopyable.h"

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

    HTTPParser() { }

    static bool gotAll() { return state_ == GotAll; }
    static bool parseRequest(Buffer* in, HTTPRequest* req);
    static bool parseRequestLine(const char* begin, const char* end, HTTPRequest** req);
private:
    static RequestParseState state_;

};

}

#endif
