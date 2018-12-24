#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <raver/http/HTTPRequest.h>

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

  HTTPParser() = default;

  ~HTTPParser() = default;

  bool gotAll() const { return state_ == GotAll; }

  bool parseRequest(Buffer* in);

  const HTTPRequest& request() const { return request_; }

 private:
  bool parseRequestLine(const char* begin, const char* end);

 private:
  RequestParseState state_{ExpectRequstLine};
  HTTPRequest request_;
};

}  // namespace raver

#endif
