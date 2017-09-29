#include "FileCache.h"
#include "Logger.h"
#include "Buffer.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
        auto file = iter->first;
        auto file_info = iter->second;
        *buf = file_info.buffer;
        cache_lock_.unlock();
        __sync_fetch_and_add(&file_info.count_pinned, 1);
    } else { // file not pinned before.
        cache_lock_.unlock();
        int fd = ::open(filename.c_str(), O_RDONLY);
        if (fd < 0) {
            LOG_SYSERR << "open file failed.";
            return ;
        }

        struct stat filestat;
        ::fstat(fd, &filestat);
        raver::Buffer* new_buf = new raver::Buffer(filestat.st_size);
        int saved_errno;
        if (new_buf->readFd(fd, &saved_errno) < 0) {
            errno = saved_errno;
            LOG_SYSERR << "read file failed.";
            ::close(fd);
            delete new_buf;
            return ;
        }

        cache_lock_.wrlock();

        int free_space = capacity_ - bytes_used_;
        if (free_space >= filestat.st_size) { // can cache file.

        } else { // no space for file, evict other files using LRU.

        }
    }
}

void FileCache::unpin(const std::string& filename)
{

}

}
