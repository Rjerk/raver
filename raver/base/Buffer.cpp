#include "Buffer.h"

namespace raver {

Buffer::Buffer(size_t sz)
    : buffer_(sz + PREPEND_SIZE),
      reader_index_(PREPEND_SIZE),
      writer_index_(PREPEND_SIZE) {
  assert(readableBytes() == 0);
  assert(writableBytes() == sz);
  assert(prependableBytes() == PREPEND_SIZE);
}

size_t Buffer::size() const { return buffer_.size(); }

size_t Buffer::readableBytes() const { return writer_index_ - reader_index_; }

size_t Buffer::writableBytes() const { return buffer_.size() - writer_index_; }

size_t Buffer::prependableBytes() const { return reader_index_; }

char* Buffer::beginWrite() { return begin() + writer_index_; }

const char* Buffer::beginRead() const { return begin() + reader_index_; }

void Buffer::hasWriten(size_t len) {
  assert(len <= writableBytes());
  writer_index_ += len;
}

void Buffer::unWrite(size_t len) {
  assert(len <= readableBytes());
  writer_index_ -= len;
}

void Buffer::append(const char* data, size_t len) {
  if (writableBytes() < len) {
    makeSpace(len);
  }
  std::copy(data, data + len, beginWrite());
  hasWriten(len);
}

void Buffer::append(const std::string& data) {
  append(data.data(), data.size());
}

void Buffer::retrieve(size_t len) {
  assert(len <= readableBytes());
  if (len < readableBytes()) {
    reader_index_ += len;
  } else {
    retrieveAll();
  }
}

void Buffer::retrieveAll() {
  reader_index_ = PREPEND_SIZE;
  writer_index_ = PREPEND_SIZE;
}

void Buffer::retrieveUntil(const char* end) {
  assert(peek() <= end);
  assert(end <= beginWrite());
  retrieve(end - peek());
}

const char* Buffer::peek() const { return begin() + reader_index_; }

const char* Buffer::findCRLF() {
  const char* crlf =
      std::search(begin() + reader_index_, beginWrite(), CRLF, CRLF + 2);
  return crlf == beginWrite() ? nullptr : crlf;
}

ssize_t Buffer::readFd(int fd, int* saved_errno) {
  char extrabuf[65536];
  struct iovec vec[2];
  size_t writable = writableBytes();
  vec[0].iov_base = beginWrite();
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);
  auto iovcount = (writable < sizeof(extrabuf)) ? 2 : 1;
  auto n = ::readv(fd, vec, iovcount);
  if (n < 0) {
    *saved_errno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    writer_index_ += n;
  } else {  // n > writable
    writer_index_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  return n;
}

char* Buffer::begin() { return &*buffer_.begin(); }

const char* Buffer::begin() const { return buffer_.data(); }

void Buffer::makeSpace(size_t len) {
  if (writableBytes() + prependableBytes() < len + PREPEND_SIZE) {
    buffer_.resize(len + writer_index_);
  } else {
    assert(PREPEND_SIZE < reader_index_);
    size_t readable = readableBytes();
    std::copy(begin() + reader_index_, begin() + writer_index_,
              begin() + PREPEND_SIZE);
    reader_index_ = PREPEND_SIZE;
    writer_index_ = reader_index_ + readable;
    assert(readable == readableBytes());
  }
}

}  // namespace raver
