#include "FileCache.h"
#include "Logger.h"
#include "Buffer.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cassert>

namespace raver {

FileCache::FileCache(size_t maxsize)
    : capacity_(maxsize)
{
}


FileCache::~FileCache()
{
}

void FileCache::pin(const std::string& filename, Buffer** buf)
{
    cache_lock_.rdlock();
    auto iter = cache_.find(filename);

    if (iter != cache_.end()) { // if find file in cache.
        // hit it, move the filename to list's head.
        files_pinned_.erase(iter->second.fileptr);
        files_pinned_.push_front(filename);

        *buf = iter->second.buffer;
        __sync_fetch_and_add(&iter->second.count_pinned, 1);

        cache_lock_.unlock();
        return ;
    }

    // not found in cache, we cache it.
    cache_lock_.unlock();

    cache_lock_.wrlock();

    // read file and get file info.
    int fd = ::open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
        LOG_SYSERR << "open file failed.";
        return ;
    }

    struct stat filestat;
    ::fstat(fd, &filestat);

    // if there are not enough space to contain a new file, unpin one (use LRU).
    while (static_cast<long>(capacity_ - bytes_used_) < filestat.st_size) {
        LOG_TRACE << "bytes_used_: " << bytes_used_;
        unpin(files_pinned_.back());

        if (bytes_used_ == 0) { // file size > capacity_.
            break;
        }
    }

    if (static_cast<long>(capacity_ - bytes_used_) < filestat.st_size) {
        LOG_ERROR << "file is too big.";
        return ; // FIXME: handle this error.
    }

    // enough space, so pin new file.
    raver::Buffer* new_buf = new raver::Buffer(filestat.st_size);
    int saved_errno;
    if (new_buf->readFd(fd, &saved_errno) < 0) {
        errno = saved_errno;
        LOG_SYSERR << "read file failed.";
        ::close(fd);
        delete new_buf;
        return ;
    }

    files_pinned_.push_front(filename);
    *buf = new_buf;
    cache_.emplace(std::make_pair(filename, FileInfo(files_pinned_.begin(), new_buf, 1)));
    //cache_[filename] = FileInfo(files_pinned_.begin(), new_buf, 1);
    bytes_used_ += new_buf->size();

    cache_lock_.unlock();
}

void FileCache::unpin(const std::string& filename)
{
    auto iter = cache_.find(filename);

    if (iter == cache_.end()) {
        LOG_ERROR << "file not found in cache.";
        return ; // FIXME: handle the error.
    }

    bytes_used_ -= iter->second.buffer->size();

    delete [] iter->second.buffer;
    files_pinned_.erase(iter->second.fileptr);
    cache_.erase(filename);
}

}
