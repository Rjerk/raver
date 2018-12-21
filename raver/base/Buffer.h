#ifndef BUFFER_H
#define BUFFER_H

#include <sys/uio.h>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include "Logger.h"

namespace raver {

const char CRLF[] = "\r\n";

class Buffer {
 public:
  static const size_t INIT_SIZE = 1024;
  static const size_t PREPEND_SIZE = 8;

  explicit Buffer(size_t sz = INIT_SIZE);

  size_t size() const;

  size_t readableBytes() const;

  size_t writableBytes() const;

  size_t prependableBytes() const;

  char* beginWrite();

  const char* beginRead() const;

  void hasWriten(size_t len);

  void unWrite(size_t len);

  void append(const char* data, size_t len);

  void append(const std::string& str);

  void append(const void* data, size_t len);

  void retrieve(size_t len);

  void retrieveAll();

  void retrieveUntil(const char* end);

  const char* peek() const;

  const char* findCRLF();

  ssize_t readFd(int fd, int* saved_errno);

 private:
  char* begin();

  const char* begin() const;

  void makeSpace(size_t len);

 private:
  std::vector<char> buffer_;
  size_t reader_index_;
  size_t writer_index_;
};

}  // namespace raver

#endif
