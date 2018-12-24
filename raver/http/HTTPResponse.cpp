#include <raver/http/HTTPResponse.h>
#include <raver/base/Buffer.h>

#include <cstring>

namespace raver {

void HTTPResponse::appendToBuffer(Buffer* out) {
  char buf[32];
  snprintf(buf, sizeof(buf), "HTTP/1.1 %u ", static_cast<unsigned>(status_code_));
  out->append(buf, strlen(buf));
  out->append(status_message_);
  out->append("\r\n", 2);

  std::string header;
  if (close_connection_) {
    header = "Connection: close\r\n";
    out->append(header);
  } else {
    snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
    out->append(buf, strlen(buf));
    header = "Connection: keep-alive\r\n";
    out->append(header);
  }

  for (auto it = headers_.cbegin(); it != headers_.end(); ++it) {
    out->append(it->first);
    out->append(": ", 2);
    out->append(it->second);
    out->append("\r\n", 2);
  }

  out->append("\r\n");
  out->append(body_);
}

}  // namespace raver
