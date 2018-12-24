#include "raver/base/FileCache.h"
#include "raver/base/Buffer.h"
#include "raver/base/Logger.h"
#include <cassert>

int main()
{
    using namespace raver;

    Buffer* buf = nullptr;
    FileCache cache(50 << 20); // cache size: 50 MB.

    cache.pin("../../www/index.html", &buf);
    assert(buf != nullptr);

    LOG_INFO << buf->readableBytes();
    LOG_INFO << buf->beginRead();
}
