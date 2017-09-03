#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <unordered_map>

namespace raver {

class HTTPResponse {
public:
    enum HTTPStatusCode {
        Unknown,
        OK200 = 200,
        MovedPermanently301 = 301,
        BadRequest400 = 400,
        NotFound404 = 404
    };

    void setStatusCode(HTTPStatusCode code)
    { status_code_ = code; }

    void setStatusMessage(const std::string& message)
    { status_message_ = message; }

    void setCloseConnection(bool on)
    { close_connection_ = on; }

    void setBody(const std::string& body)
    { body_ = body; }

private:

    HTTPStatusCode status_code_;
    std::string status_message_;
    bool close_connection_;

    using HeaderMap = std::unordered_map<std::string, std::string>;
    HeaderMap headers_;

    std::string body_;

};

}

#endif
