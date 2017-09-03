#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include <vector>
#include <algorithm>

namespace raver {

constexpr size_t INIT_SIZE = 1024;

const char CRLF[] = "\r\n";

class Buffer {
public:
    explicit Buffer(size_t sz = INIT_SIZE)
        : buffer_(sz), reader_index_(0), writer_index_(0)
    {
    }

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
        makeSpace(len); // ensure len bytes to write.
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
        makeSpace(len);
        std::copy(data, data+len, beginWrite());
        writer_index_ += len;
    }

    void append(const void* data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    void retrieve(size_t len)
    {
        if (len < readableBytes()) {
            reader_index_ += len;
        } else {
            reader_index_ = 0;
            writer_index_ = 0;
        }
    }

    void retrieveUntil(const char* end)
    {
        if (peek() <= end && end <= beginWrite()) {
            retrieve(end - peek());
        }
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

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return buffer_.data(); }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len) {
            buffer_.resize(len);
        } else {
            auto readable = readableBytes();
            std::copy(begin()+reader_index_, begin()+writer_index_, begin());
            reader_index_ = 0;
            writer_index_ = reader_index_ + readable;
        }
    }
private:
    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;
};


}

#endif
