#include "HTTPParser.h"
#include "HTTPRequest.h"
#include "../base/Buffer.h"

namespace raver {

bool HTTPParser::parseRequest(Buffer* buf, HTTPRequest* req)
{
    LOG_TRACE << "";
    bool ok = true;
    bool has_more = true;

    while (has_more) {
        if (state_ == ExpectRequstLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = parseRequestLine(buf->peek(), crlf, &req);
                if (ok) {
                    buf->retrieveUntil(crlf+2);
                    state_ = ExpectHeaders;
                } else {
                    has_more = false;
                }
            } else {
                has_more = false;
            }
        } else if (state_ == ExpectHeaders) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    req->addHeader(buf->peek(), colon, crlf);
                    buf->retrieveUntil(crlf+2);
                } else { // empty line.
                    if (buf->findCRLF()) {
                        state_ = ExpectBody;
                        buf->retrieve(2); // CRLF.
                    } else {
                        has_more = false;
                    }
                }
            } else {
                has_more = false;
            }
        } else if (state_ == ExpectBody) {
            req->setBody(std::string(buf->peek(), static_cast<const char*>(buf->beginWrite())));
            state_ = GotAll;
            has_more = false;
        }
    }
    return ok;
}

bool HTTPParser::parseRequestLine(const char* begin, const char* end, HTTPRequest** req)
{
    bool succeed = false;
    const char* beg = begin;
    const char* space = std::find(beg, end, ' ');

    if (space != end && (*req)->setMethod(beg, space)) {
        beg = space + 1;
        space = std::find(beg, end, ' ');

        if (space != end) {
            const char* query = std::find(beg, space, '?');
            if (query != space) {
                (*req)->setPath(std::string(beg, query));
                (*req)->setQuery(std::string(query, space));
            } else { // no query part.
                (*req)->setPath(std::string(beg, space));
            }
        }

        beg = space;
        const char* http = std::find(beg, end, 'H');
        succeed = (end - http == 8) && (std::equal(http, end-1, "HTTP/1."));
        if (succeed) {
            if (*(end-1) == '1') {
                (*req)->setVersion(HTTPRequest::Version::HTTP11);
            } else if (*(end-1) == '0') {
                (*req)->setVersion(HTTPRequest::Version::HTTP10);
            } else {
                succeed = false;
            }
        }
    }
    return succeed;
}

}
