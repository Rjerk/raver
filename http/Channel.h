#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <mutex>

namespace raver {

class IOManager;
class ThreadPool;

using Callback = std::function<void ()>;
using MutexGuard = std::lock_guard<std::mutex>;

class Channel {
    friend class IOManager;
public:
    Channel(IOManager* io, int fd, const Callback& readcb, const Callback& writecb);
    ~Channel();

    void read();

    void write();

    void setReadCallback(const Callback& readcb) { readcb_ = readcb; }
    void setReadCallback(Callback&& readcb) { readcb_ = std::move(readcb); }
    void setWriteCallback(const Callback& writecb) { writecb_ = writecb; }
    void setWriteCallback(Callback&& writecb) { writecb_ = std::move(writecb); }

    int fd() const { return fd_; }
private:
    ThreadPool* getPool();

private:
    int fd_;
    IOManager* io_; // not own it.
    bool closed_fd_;
    Callback readcb_;
    Callback writecb_;
    std::mutex mtx_;
    bool can_read_;
    bool can_write_;
    bool waiting_read_;
    bool waiting_write_;
};

}

#endif
