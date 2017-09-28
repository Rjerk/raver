#ifndef FILE_CACHE_H
#define FILE_CACHE_H

#include <unordered_map>
#include <list>
#include "RWMutex.h"

namespace base {

class Buffer;

class FileCache : noncopyable {
public:
    explicit FileCache(size_t max_size);

    ~FileCache();

    void pin(const std::string& filename, Buffer** buf);

    void unpin(const std::string& filename);

private:
    using File = std::string;
    using PinnedFiles = std::list<File>;
    using FileInfo = struct FileInfo {
        FileInfo(PinnedFiles::iterator file, Buffer* buf, int count)
                    : fileptr(file), buffer(buf), count_pinned(count) { }
        PinnedFiles::iterator fileptr;
        Buffer* buffer; // not own it.
        int count_pinned;
    };
    using FileMap  = std::unordered_map<File, FileInfo>;


    PinnedFiles files_pinned_;
    FileMap cache_;
    RWMutex cache_lock_;

    size_t capacity_; // bytes.
    size_t bytes_used_;
};


}



#endif
