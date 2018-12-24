#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <unordered_map>

namespace raver {

class HTTPRequest {
 public:
  enum class Method { Invalid, Get, Post, Head, Put, Delete };
  enum class Version { Unknown, HTTP10, HTTP11 };

  HTTPRequest() = default;

  void clear();

  void addHeader(const char* begin, const char* colon, const char* end);

  bool setMethod(const char* beg, const char* end);

  void setVersion(Version version) { version_ = version; }

  void setPath(std::string&& path) { path_ = std::move(path); }

  void setQuery(std::string&& query) { query_ = std::move(query); }

  void setBody(std::string&& body) { body_ = std::move(body); }

  Method getMethod() const { return method_; }
  Version getVersion() const { return version_; }
  const std::string& getPath() const { return path_; }
  const std::string& getQuery() const { return query_; }
  std::string getHeader(const std::string& field) const;

 private:
  Method method_{Method::Invalid};
  Version version_{Version::Unknown};
  std::string path_;
  std::string query_;

  using HeaderMap = std::unordered_map<std::string, std::string>;
  HeaderMap headers_;

  std::string body_;
};

}  // namespace raver

#endif
