#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sys/uio.h>
#include <cassert>

#include "Logger.h"

namespace raver {


const char CRLF[] = "\r\n";

class Buffer {
public:
    static const size_t INIT_SIZE = 1024;
    static const size_t PREPEND_SIZE = 8;

    explicit Buffer(size_t sz = INIT_SIZE)
        : buffer_(sz + PREPEND_SIZE),
          reader_index_(PREPEND_SIZE), writer_index_(PREPEND_SIZE)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == sz);
        assert(prependableBytes() == PREPEND_SIZE);
    }

    size_t size() const
    { return buffer_.size(); }

    size_t readableBytes() const
    { return writer_index_ - reader_index_; }

    size_t writableBytes() const
    { return buffer_.size() - writer_index_; }

    size_t prependableBytes() const
    { return reader_index_; }

    char* beginWrite()
    { return begin() + writer_index_; }

    const char* beginRead() const
    { return begin() + reader_index_; }

    void hasWriten(size_t len)
    {
        assert(len <= writableBytes());
        writer_index_ += len;
    }

    void unWriten(size_t len)
    {
        if (len <= readableBytes()) {
            writer_index_ -= len;
        } else {
            writer_index_ -= readableBytes();
        }
    }

    void append(const char* data, size_t len)
    {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        std::copy(data, data+len, beginWrite());
        writer_index_ += len;
    }

    void append(const std::string& str)
    {
        append(str.data(), str.size());
    }

    void append(const void* data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    void retrieve(size_t len)
    {
        assert(len < readableBytes());
        if (len < readableBytes()) {
            reader_index_ += len;
        } else {
            reader_index_ = PREPEND_SIZE;
            writer_index_ = PREPEND_SIZE;
        }
    }

    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    const char* peek() const
    {
        return begin() + reader_index_;
    }

    const char* findCRLF()
    {
        const char* crlf = std::search(begin()+reader_index_, beginWrite(), CRLF, CRLF+2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    ssize_t readFd(int fd, int* saved_errno)
    {
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
        } else { // n > writable
            writer_index_ = buffer_.size();
            append(extrabuf, n - writable);
        }
        return n;
    }

    void clear()
    {
        buffer_.clear();
        reader_index_ = PREPEND_SIZE;
        writer_index_ = PREPEND_SIZE;
    }

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return buffer_.data(); }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + PREPEND_SIZE) {
            buffer_.resize(len + writer_index_);
        } else {
            assert(PREPEND_SIZE < reader_index_);
            size_t readable = readableBytes();
            std::copy(begin()+reader_index_, begin()+writer_index_, begin() + PREPEND_SIZE);
            reader_index_ = PREPEND_SIZE;
            writer_index_ = reader_index_ + readable;
            assert(readable == readableBytes());
        }
    }
private:
    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;
};


}

#endif
